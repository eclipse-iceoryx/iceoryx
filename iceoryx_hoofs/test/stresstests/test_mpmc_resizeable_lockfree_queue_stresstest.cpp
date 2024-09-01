// Copyright (c) 2020, 2022 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_hoofs/testing/test.hpp"

#include "iox/atomic.hpp"
#include "iox/detail/mpmc_resizeable_lockfree_queue.hpp"

// Remark: It would be nice to have way to configure the (maximum) runtime in a general way.

using namespace ::testing;

#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"

#include <list>
#include <numeric>
#include <random>
#include <thread>
#include <utility>
#include <vector>

namespace
{
struct Data
{
    //NOLINTNEXTLINE(bugprone-easily-swappable-parameters) This is okay since it is limited to the stress test
    explicit Data(uint32_t id = 0, uint64_t count = 0)
        : id(id)
        , count(count)
    {
    }

    uint32_t id{0};
    uint64_t count{0};
};

// global barrier is not ideal and should be changed later to a barrier per test
// (requires lambdas and/or modification of the functions run by the threads)
Barrier g_barrier;

using CountArray = std::vector<iox::concurrent::Atomic<uint64_t>>;

template <typename Queue>
void producePeriodic(Queue& queue, const uint32_t id, CountArray& producedCount, iox::concurrent::Atomic<bool>& run)
{
    g_barrier.notify();

    const auto n = producedCount.size();
    Data d(id, 0U);
    while (run.load(std::memory_order_relaxed))
    {
        if (queue.tryPush(d))
        {
            producedCount[static_cast<size_t>(d.count)].fetch_add(1U, std::memory_order_relaxed);
            d.count = (d.count + 1U) % n;
        }
    }
}

template <typename Queue>
void consume(Queue& queue, CountArray& consumedCount, iox::concurrent::Atomic<bool>& run)
{
    g_barrier.notify();

    // stop only when we are not supposed to run anymore AND the queue is empty
    while (run.load(std::memory_order_relaxed) || !queue.empty())
    {
        const auto popped = queue.pop();
        if (popped.has_value())
        {
            const auto& value = popped.value();
            consumedCount[static_cast<size_t>(value.count)].fetch_add(1U, std::memory_order_relaxed);
        }
    }
}

template <typename Queue>
void produceMonotonic(Queue& queue, const uint32_t id, iox::concurrent::Atomic<bool>& run)
{
    g_barrier.notify();

    Data d(id, 1U);
    while (run.load(std::memory_order_relaxed))
    {
        while (!queue.tryPush(d) && run.load(std::memory_order_relaxed))
        {
        }
        ++d.count;
    }
}

//NOLINTBEGIN(bugprone-easily-swappable-parameters) This is okay since it is limited to the stress test
template <typename Queue>
void consumeAndCheckOrder(Queue& queue,
                          const uint32_t maxId,
                          iox::concurrent::Atomic<bool>& run,
                          iox::concurrent::Atomic<bool>& orderOk)
//NOLINTEND(bugprone-easily-swappable-parameters)
{
    g_barrier.notify();

    // note that the producers start sending with count 1,
    // hence setting last count to 0 does not lead to false negative checks
    std::vector<uint64_t> lastCount(maxId + 1, 0U);
    const auto numCounts = lastCount.size();

    while (run.load(std::memory_order_relaxed) || !queue.empty())
    {
        const auto popped = queue.pop();
        if (popped.has_value())
        {
            const auto& value = popped.value();

            if (value.id >= 0 && value.id < numCounts)
            {
                const auto& count = lastCount[value.id];
                if (count >= value.count)
                {
                    // last count received with this id is equal or larger,
                    // which should not be the case if the counts are monotonic
                    // and indicates an error
                    orderOk.store(false, std::memory_order_relaxed);
                }
                lastCount[value.id] = value.count;
            }
            else
            {
                // the id is invalid (out of range)
                // which means the queue has corrupted the data somehow
                // and the test should fail
                orderOk.store(false, std::memory_order_relaxed);
            }
        }
    }
}

// alternates between push and pop
template <typename Queue>
void work(Queue& queue, uint32_t id, iox::concurrent::Atomic<bool>& run)
{
    g_barrier.notify();

    // technically one element suffices if we alternate,
    // but if we want to test other push/pop patterns a list is useful
    std::list<Data> poppedValues;
    bool doPop = true;

    while (run)
    {
        if (doPop)
        {
            auto popped = queue.pop();
            if (popped.has_value())
            {
                poppedValues.push_back(popped.value());
                doPop = false;
            }
        }
        else
        {
            // try a push (we know the list is not empty since doPop is false)
            auto value = poppedValues.front();
            if (queue.tryPush(value))
            {
                poppedValues.pop_front();
                doPop = true;
            }
        }
    }

    // push the remaining items back into the queue
    for (auto& value : poppedValues)
    {
        value.id = id;
        while (!queue.tryPush(value))
        {
        }
    }
}

// randomly chooses between push and pop
// popProbability essentially controls whether the queue tends to be full or empty on average
template <typename Queue>
//NOLINTNEXTLINE(readability-function-size) This is okay since it is limited to the stress test
void randomWork(Queue& queue,
                uint32_t id,
                iox::concurrent::Atomic<bool>& run,
                uint64_t numItems,
                int& overflowCount,
                std::list<Data>& items,
                double popProbability = 0.5)
{
    g_barrier.notify();

    Data value;
    value.id = id;

    // populate the list with capacity unique items (with this workers id)
    for (value.count = 0; value.count < numItems; ++value.count)
    {
        items.push_back(value);
    }

    bool doPop = false;
    std::default_random_engine rng{std::random_device()()};
    std::uniform_real_distribution<double> dist{0, 1};

    overflowCount = 0;
    while (run)
    {
        if (doPop)
        {
            auto popped = queue.pop();
            if (popped.has_value())
            {
                items.push_back(popped.value());
            }
        }
        else
        {
            // we know the list is not empty since doPop is false
            value = items.front();
            auto overflow = queue.push(value);
            if (overflow.has_value())
            {
                // overflow, store the overflow item in the local list
                items.push_back(overflow.value());
                overflowCount++;
            }

            items.pop_front(); // we pushed the value but we keep it anyway to produce overflows
        }

        if (items.empty())
        {
            // cannot push, so we choose pop
            doPop = true;
            continue;
        }

        // choose next action: push or pop?
        doPop = dist(rng) <= popProbability;
    }
}

template <typename Queue>
//NOLINTNEXTLINE(readability-function-size) This is okay since it is limited to the stress test
void changeCapacity(Queue& queue,
                    iox::concurrent::Atomic<bool>& run,
                    std::list<Data>& items,
                    std::vector<uint64_t>& capacities,
                    uint64_t& numChanges)
{
    g_barrier.notify();

    const uint64_t n = capacities.size(); // number of different capacities
    auto k = static_cast<int64_t>(n);     // index of current capacity to be used
    bool incrementK = false;              // states if k is incremented or decremented by 1
    numChanges = 0;                       // number of capacity changes performed

    // capacities will contain a number of pre generated capacities to switch between,
    // ordered from lowest to highest
    // starting with the highest (at capacities[k], k=n-1) a new capacity is set,
    // preserving potentially removed elements in items
    // afterwards k is decremented (k+=d with d=-1) and the next capacity is set, this continues until k=0
    // when k=0, the order is reversed (d=1) and we increase the capacities until we reach k=n-1 again
    // this continues until the thread is stopped

    auto removeHandler = [&](const auto& value) { items.emplace_back(std::move(value)); };

    while (run)
    {
        // go forward and backward in the capacities array to select the next capacity
        if (incrementK)
        {
            ++k;
        }
        else
        {
            --k;
        }

        if (k < 0)
        {
            k = 1;
            incrementK = true;
        }
        else if (static_cast<uint64_t>(k) >= n)
        {
            k = static_cast<int64_t>(n - 1);
            incrementK = false;
        }

        if (queue.setCapacity(capacities[static_cast<size_t>(k)], removeHandler))
        {
            ++numChanges;
        }

        // push items back before changing capacity again
        // (we do not want to discard them in the test to be able to count later
        // nor do we want to have them only in the thread running this method thread)

        while (run && !items.empty())
        {
            if (queue.tryPush(items.front()))
            {
                items.pop_front();
            }
        }
    }
}


template <typename Config>
class MpmcResizeableLockFreeQueueStressTest : public ::testing::Test
{
  protected:
    MpmcResizeableLockFreeQueueStressTest() = default;

    void SetUp() override
    {
        // reduce capacity if desired before running the tests
        if (Config::DynamicCapacity < Config::Capacity)
        {
            sut.setCapacity(Config::DynamicCapacity);
        }

        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    void TearDown() override
    {
    }

    using Queue = typename Config::QueueType;
    Queue sut;

    std::chrono::seconds runtime{3};

  private:
    const iox::units::Duration m_fatalTimeout = 60_s + iox::units::Duration::fromSeconds(runtime.count());
    Watchdog m_watchdog{m_fatalTimeout};
};

template <typename ElementType, uint64_t Capacity_, uint64_t DynamicCapacity_ = Capacity_>
struct Config
{
    static constexpr uint64_t Capacity = Capacity_;
    static constexpr uint64_t DynamicCapacity = DynamicCapacity_;
    static_assert(DynamicCapacity <= Capacity, "DynamicCapacity can be at most Capacity");


    using QueueType = iox::concurrent::MpmcResizeableLockFreeQueue<ElementType, Capacity>;
};

template <typename ElementType, uint64_t Capacity>
using Full = Config<ElementType, Capacity>;

template <typename ElementType, uint64_t Capacity>
using AlmostFull = Config<ElementType, Capacity, (Capacity > 1U) ? (Capacity - 1U) : Capacity>;

template <typename ElementType, uint64_t Capacity>
using HalfFull = Config<ElementType, Capacity, (Capacity > 1U) ? (Capacity / 2U) : Capacity>;

// test different queue sizes with full and reduced dynamic capacity
constexpr uint64_t Small = 10U;
constexpr uint64_t Medium = 1000U;
constexpr uint64_t Large = 1000000U;

using Full1 = Full<Data, 1>;
using Full2 = Full<Data, Small>;
using Full3 = Full<Data, Medium>;
using Full4 = Full<Data, Large>;

using AlmostFull1 = AlmostFull<Data, Small>;
using AlmostFull2 = AlmostFull<Data, Medium>;
using AlmostFull3 = AlmostFull<Data, Large>;

using HalfFull1 = HalfFull<Data, Small>;
using HalfFull2 = HalfFull<Data, Medium>;
using HalfFull3 = HalfFull<Data, Large>;

/// @code
/// typedef ::testing::
///    Types<Full1, Full2, Full3, Full4, AlmostFull1, AlmostFull2, AlmostFull3, HalfFull1, HalfFull2, HalfFull3>
///        TestConfigs;
/// @endcode
typedef ::testing::Types<HalfFull2> TestConfigs;

TYPED_TEST_SUITE(MpmcResizeableLockFreeQueueStressTest, TestConfigs, );

///@brief Tests concurrent operation of multiple producers and consumers
///       with respect to completeness of the data, i.e. nothing is lost.
TYPED_TEST(MpmcResizeableLockFreeQueueStressTest, MultiProducerMultiConsumerCompleteness)
{
    ::testing::Test::RecordProperty("TEST_ID", "9640d068-5c9f-4bc4-b4a0-c0a2225c15ed");
    using Queue = typename TestFixture::Queue;
    auto& queue = this->sut;

    iox::concurrent::Atomic<bool> run{true};

    const int numProducers{4};
    const int numConsumers{4};
    g_barrier.reset(numProducers + numConsumers);

    // the producers will only send items with a count 0<=count<cycleLength
    // and wrap around modulo this cycleLength (bounded to be able to count arrived data in an array)
    // unfortunately we cannot really check out of order arrival this way, since
    // the sent counts are not monotonic themselves due to the wraparound

    const uint64_t cycleLength{1000U};
    CountArray producedCount(cycleLength);
    CountArray consumedCount(cycleLength);

    // cannot be done with the vector ctor for atomics
    for (size_t i = 0; i < cycleLength; ++i)
    {
        producedCount[i].store(0U, std::memory_order_relaxed);
        consumedCount[i].store(0U, std::memory_order_relaxed);
    }

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    producers.reserve(numProducers);
    for (int id = 0; id < numProducers; ++id)
    {
        producers.emplace_back(producePeriodic<Queue>, std::ref(queue), id, std::ref(producedCount), std::ref(run));
    }

    consumers.reserve(numConsumers);
    for (int id = 0; id < numConsumers; ++id)
    {
        consumers.emplace_back(consume<Queue>, std::ref(queue), std::ref(consumedCount), std::ref(run));
    }

    g_barrier.wait();
    std::this_thread::sleep_for(std::chrono::seconds(this->runtime));
    run = false;

    for (auto& producer : producers)
    {
        producer.join();
    }
    for (auto& consumer : consumers)
    {
        consumer.join();
    }

    // necessary to avoid missing a produced value on consumer side
    while (const auto popped = queue.pop())
    {
        const auto& value = popped.value();
        consumedCount[static_cast<size_t>(value.count)].fetch_add(1U, std::memory_order_relaxed);
    }

    // verify counts
    for (size_t i = 0; i < cycleLength; ++i)
    {
        EXPECT_EQ(producedCount[i].load(), consumedCount[i].load());
    }
}


/// @brief Tests concurrent operation of multiple producers and consumers
///       with respect to order of the data (monotonic, FIFO).
/// @note this cannot be done easily together with completeness and limited memory
TYPED_TEST(MpmcResizeableLockFreeQueueStressTest, MultiProducerMultiConsumerOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "5a6e3e6b-7cd9-4079-a9e8-7a849ea3dfe9");
    using Queue = typename TestFixture::Queue;
    auto& queue = this->sut;

    iox::concurrent::Atomic<bool> run{true};

    const int numProducers{4};
    const int numConsumers{4};
    g_barrier.reset(numProducers + numConsumers);

    // need only one variable, any consumer that detects an error will set it to false
    // and no consumer will ever set it to true again
    iox::concurrent::Atomic<bool> orderOk{true};

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    producers.reserve(numProducers);
    for (int id = 0; id < numProducers; ++id)
    {
        producers.emplace_back(produceMonotonic<Queue>, std::ref(queue), id, std::ref(run));
    }

    consumers.reserve(numConsumers);
    for (int id = 0; id < numConsumers; ++id)
    {
        consumers.emplace_back(
            consumeAndCheckOrder<Queue>, std::ref(queue), numProducers - 1, std::ref(run), std::ref(orderOk));
    }

    g_barrier.wait();
    std::this_thread::sleep_for(std::chrono::seconds(this->runtime));
    run = false;

    for (auto& producer : producers)
    {
        producer.join();
    }
    for (auto& consumer : consumers)
    {
        consumer.join();
    }

    EXPECT_TRUE(orderOk.load(std::memory_order_relaxed));
}

///@brief Tests concurrent operation of multiple hybrid producer/consumer threads.
/// The tests initializes a queue full of distinct (unique) elements
/// and each thread alternates between pop and push, only pushing what he has previously popped.
/// The test runs for some specified time and upon completion each thread pushes every consumed
/// data item back into the queue.
/// Finally it is checked whether the queue still contains all elements it was initialized with
///(likely in a different order).
TYPED_TEST(MpmcResizeableLockFreeQueueStressTest, HybridMultiProducerMultiConsumer)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b5c7dc4-6e9a-4ac4-b2fc-6bd6dfb7ee1f");
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    const int numThreads = 32;
    g_barrier.reset(numThreads);

    auto capacity = q.capacity();

    // fill the queue
    Data d;
    for (size_t i = 0; i < capacity; ++i)
    {
        d.count = i;
        while (!q.tryPush(d))
        {
        }
    }

    iox::concurrent::Atomic<bool> run{true};

    std::vector<std::thread> threads;

    threads.reserve(numThreads);
    for (uint32_t id = 1; id <= numThreads; ++id)
    {
        threads.emplace_back(work<Queue>, std::ref(q), id, std::ref(run));
    }

    g_barrier.wait();
    std::this_thread::sleep_for(std::chrono::seconds(this->runtime));
    run = false;

    for (auto& thread : threads)
    {
        thread.join();
    }

    // check whether all elements are there, but there is no specific ordering we can expect
    std::vector<uint64_t> count(static_cast<size_t>(capacity), 0);
    auto popped = q.pop();
    while (popped.has_value())
    {
        count[static_cast<size_t>(popped.value().count)]++;
        popped = q.pop();
    }

    bool testResult = true;
    for (size_t i = 0; i < capacity; ++i)
    {
        if (count[i] != 1) // missing or duplicate elements indicate an error
        {
            testResult = false;
            break;
        }
    }

    EXPECT_EQ(testResult, true);
}

///@brief Tests concurrent operation of multiple hybrid producer/consumer threads
/// which use potentially overflowing pushes.
/// The tests initializes a local list of distinct elements for each thread.
/// The queue is also filled with distinct elements to ensure we will have an overflow.
/// Each thread chooses randomly between push and pop (preference is controllable, to make overlflow more or less
/// likely).
/// The test runs for some specified time and upon completion it is checked that
/// aggregated over the queue and the local lists of each thread
/// all elements occur exactly as often as there are threads + 1 (i.e. nothing was lost, the +1 is
/// due to the initial values in the queue itself).
TYPED_TEST(MpmcResizeableLockFreeQueueStressTest, HybridMultiProducerMultiConsumer0verflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "57516ebd-e994-42c8-813c-613c61f2410f");
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    std::chrono::seconds runtime(10);
    const uint32_t numThreads = 32;
    const double popProbability = 0.45; // tends to overflow
    const auto capacity = q.capacity();

    iox::concurrent::Atomic<bool> run{true};

    std::vector<std::thread> threads;
    std::vector<int> overflowCount(numThreads);
    std::vector<std::list<Data>> itemVec(numThreads);

    g_barrier.reset(numThreads);

    // fill the queue
    Data d;
    for (size_t i = 0; i < capacity; ++i)
    {
        d.count = i;
        while (!q.tryPush(d))
        {
        }
    }

    threads.reserve(numThreads);
    for (uint32_t id = 1; id <= numThreads; ++id)
    {
        threads.emplace_back(randomWork<Queue>,
                             std::ref(q),
                             id,
                             std::ref(run),
                             capacity,
                             std::ref(overflowCount[id - 1]),
                             std::ref(itemVec[id - 1]),
                             popProbability);
    }

    g_barrier.wait();
    std::this_thread::sleep_for(std::chrono::seconds(runtime));

    run = false;

    for (auto& thread : threads)
    {
        thread.join();
    }

    // we expect at least one overflow in the test (since the queue is full in the beginning)
    // we cannot expect one overflow in each thread due to thread scheduling
    auto numOverflows = std::accumulate(overflowCount.begin(), overflowCount.end(), 0LL);
    EXPECT_GT(numOverflows, 0LL);

    // check whether all elements are there, but there is no specific ordering we can expect
    // items are either in the local lists or the queue, in total we expect each count numThreads times
    // and for each count each id exactly once

    std::vector<std::vector<int>> count(static_cast<size_t>(capacity), std::vector<int>(numThreads + 1, 0));
    auto popped = q.pop();
    while (popped.has_value())
    {
        const auto& value = popped.value();
        count[static_cast<size_t>(value.count)][value.id]++;
        popped = q.pop();
    }

    // check the local lists
    for (auto& items : itemVec)
    {
        for (auto& item : items)
        {
            count[static_cast<size_t>(item.count)][item.id]++;
        }
    }

    bool testResult = true;
    for (size_t i = 0; i < capacity; ++i)
    {
        // we expect each data item exactly numThreads + 1 times,
        // the extra one is for the initially full queue
        // and each count appears for all ids exactly ones
        for (uint32_t j = 0; j <= numThreads; ++j)
        {
            if (count[i][j] != 1)
            {
                testResult = false;
                break;
            }
        }
    }

    EXPECT_EQ(testResult, true);
}

/// @brief As the test before, but with an additional thread that periodically changes the capacity
/// again it is checked that nothing is lost or created by accident.
/// @note the tests are getting quite complicated but the complex setup is unavoidable
/// in order to test the general case under load.
TYPED_TEST(MpmcResizeableLockFreeQueueStressTest, HybridMultiProducerMultiConsumer0verflowWithCapacityChange)
{
    ::testing::Test::RecordProperty("TEST_ID", "6421f32a-a1f7-4fe2-978f-6ef2005e0cc9");
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    const int numThreads = 32;
    const double popProbability = 0.45; // tends to overflow
    const auto capacity = q.capacity();

    iox::concurrent::Atomic<bool> run{true};

    std::vector<std::thread> threads;
    std::vector<int> overflowCount(numThreads);
    std::vector<std::list<Data>> itemVec(numThreads + 1);

    g_barrier.reset(numThreads + 1);

    // define capacities to cycle between
    std::vector<uint64_t> capacities;
    auto maxCapacity = q.maxCapacity();
    for (uint64_t c = 1; c < maxCapacity; c *= 2)
    {
        capacities.push_back(c);
    }
    capacities.push_back(maxCapacity);

    // fill the queue
    Data d;
    for (uint64_t i = 0; i < capacity; ++i)
    {
        d.count = i;
        while (!q.tryPush(d))
        {
        }
    }

    threads.reserve(numThreads);
    for (uint32_t id = 1; id <= numThreads; ++id)
    {
        threads.emplace_back(randomWork<Queue>,
                             std::ref(q),
                             id,
                             std::ref(run),
                             capacity,
                             std::ref(overflowCount[id - 1]),
                             std::ref(itemVec[id - 1]),
                             popProbability);
    }

    uint32_t id = numThreads + 1U;
    uint64_t numChanges{0};
    threads.emplace_back(changeCapacity<Queue>,
                         std::ref(q),
                         std::ref(run),
                         std::ref(itemVec[id - 1]),
                         std::ref(capacities),
                         std::ref(numChanges));

    g_barrier.wait();
    std::this_thread::sleep_for(std::chrono::seconds(this->runtime));

    run = false;

    for (auto& thread : threads)
    {
        thread.join();
    }

    // we expect at least one overflow in the test (since the queue is full in the beginning)
    // we cannot expect one overflow in each thread due to thread scheduling
    const auto numOverflows = std::accumulate(overflowCount.begin(), overflowCount.end(), 0ULL);
    EXPECT_GT(numOverflows, 0ULL);
    EXPECT_GT(numChanges, 0ULL);

    // check whether all elements are there, but there is no specific ordering we can expect
    // items are either in the local lists or the queue, in total we expect each count numThreads times
    // and for each count each id exactly once

    std::vector<std::vector<int>> count(static_cast<size_t>(capacity), std::vector<int>(numThreads + 1, 0));
    auto popped = q.pop();
    while (popped.has_value())
    {
        const auto& value = popped.value();
        count[static_cast<size_t>(value.count)][value.id]++;
        popped = q.pop();
    }

    // check the local lists
    for (auto& items : itemVec)
    {
        for (auto& item : items)
        {
            count[static_cast<size_t>(item.count)][item.id]++;
        }
    }

    bool testResult = true;
    for (size_t i = 0; i < capacity; ++i)
    {
        // we expect each data item exactly numThreads + 1 times,
        // the extra one is for the initially full queue
        // and each count appears for all ids exactly ones
        for (size_t j = 0; j <= numThreads; ++j)
        {
            if (count[i][j] != 1)
            {
                testResult = false;
                break;
            }
        }
    }

    EXPECT_EQ(testResult, true);
}

} // namespace
