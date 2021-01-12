// Copyright (c) 2020 Apex.AI Inc. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_utils/concurrent/resizeable_lockfree_queue.hpp"

// Remark: It would be nice to have way to configure the (maximum) runtime in a general way.

using namespace ::testing;

#include <array>
#include <atomic>
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
    Data(int32_t id = 0, uint64_t count = 0)
        : id(id)
        , count(count)
    {
    }

    uint32_t id{0};
    uint64_t count{0};
};

using CountArray = std::vector<std::atomic<uint64_t>>;

template <typename Queue>
void producePeriodic(Queue& queue, const int32_t id, CountArray& producedCount, std::atomic_bool& run)
{
    const auto n = producedCount.size();
    Data d(id, 0U);
    while (run.load(std::memory_order_relaxed))
    {
        if (queue.tryPush(d))
        {
            producedCount[d.count].fetch_add(1U, std::memory_order_relaxed);
            d.count = (d.count + 1U) % n;
        }
    }
}

template <typename Queue>
void consume(Queue& queue, CountArray& consumedCount, std::atomic_bool& run)
{
    // stop only when we are not supposed to run anymore AND the queue is empty
    while (run.load(std::memory_order_relaxed) || !queue.empty())
    {
        const auto popped = queue.pop();
        if (popped.has_value())
        {
            const auto& value = popped.value();
            consumedCount[value.count].fetch_add(1U, std::memory_order_relaxed);
        }
    }
}

template <typename Queue>
void produceMonotonic(Queue& queue, const int32_t id, std::atomic_bool& run)
{
    Data d(id, 1U);
    while (run.load(std::memory_order_relaxed))
    {
        while (!queue.tryPush(d) && run.load(std::memory_order_relaxed))
        {
        }
        ++d.count;
    }
}

template <typename Queue>
void consumeAndCheckOrder(Queue& queue, const int32_t maxId, std::atomic_bool& run, std::atomic_bool& orderOk)
{
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
void work(Queue& queue, int32_t id, std::atomic<bool>& run)
{
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
void randomWork(Queue& queue,
                int32_t id,
                std::atomic<bool>& run,
                uint64_t numItems,
                int& overflowCount,
                std::list<Data>& items,
                double popProbability = 0.5)
{
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
        doPop = (dist(rng) <= popProbability) ? true : false;
    }
}

template <typename Queue>
void changeCapacity(Queue& queue,
                    std::atomic<bool>& run,
                    std::list<Data>& items,
                    std::vector<uint64_t>& capacities,
                    uint64_t& numChanges)
{
    const int32_t n = capacities.size(); // number of different capacities
    int32_t k = n;                       // index of current capacity to be used
    int32_t d = -1;                      // increment delta of the index k, will be 1 or -1
    numChanges = 0;                      // number of capacity changes performed

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
        k += d;
        if (k < 0)
        {
            k = 1;
            d = 1;
        }
        else if (k >= n)
        {
            k = n - 1;
            d = -1;
        }

        if (queue.setCapacity(capacities[k], removeHandler))
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
class ResizeableLockFreeQueueStressTest : public ::testing::Test
{
  protected:
    ResizeableLockFreeQueueStressTest() = default;

    ~ResizeableLockFreeQueueStressTest()
    {
    }

    void SetUp()
    {
        // reduce capacity if desired before running the tests
        if (Config::DynamicCapacity < Config::Capacity)
        {
            sut.setCapacity(Config::DynamicCapacity);
        }
    }

    void TearDown()
    {
    }

    using Queue = typename Config::QueueType;
    Queue sut;

    std::chrono::seconds runtime{3};
};

template <typename ElementType, uint64_t Capacity_, uint64_t DynamicCapacity_ = Capacity_>
struct Config
{
    static constexpr uint64_t Capacity = Capacity_;
    static constexpr uint64_t DynamicCapacity = DynamicCapacity_;
    static_assert(DynamicCapacity <= Capacity, "DynamicCapacity can be at most Capacity");


    using QueueType = iox::concurrent::ResizeableLockFreeQueue<ElementType, Capacity>;
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

/// @todo these should be activated but each test takes a lot of time, occupying the CI servers
/// need separate stress test targets and policy to run them on CI,
/// currently only activate one suitable general configuration
/// for this reason some less important tests are disabled for now
/// @code
/// typedef ::testing::
///    Types<Full1, Full2, Full3, Full4, AlmostFull1, AlmostFull2, AlmostFull3, HalfFull1, HalfFull2, HalfFull3>
///        TestConfigs;
/// @endcode
typedef ::testing::Types<HalfFull2> TestConfigs;

/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(ResizeableLockFreeQueueStressTest, TestConfigs);
#pragma GCC diagnostic pop


///@brief Tests concurrent operation of multiple producers and consumers
///       with respect to completeness of the data, i.e. nothing is lost.
TYPED_TEST(ResizeableLockFreeQueueStressTest, multiProducerMultiConsumerCompleteness)
{
    using Queue = typename TestFixture::Queue;
    auto& queue = this->sut;

    std::atomic_bool run{true};

    const int numProducers{4};
    const int numConsumers{4};

    // the producers will only send items with a count 0<=count<cycleLength
    // and wrap around modulo this cycleLength (bounded to be able to count arrived data in an array)
    // unfortunately we cannot really check out of order arrival this way, since
    // the sent counts are not monotonic themselves due to the wraparound

    const int cycleLength{1000};
    CountArray producedCount(cycleLength);
    CountArray consumedCount(cycleLength);

    // cannot be done with the vector ctor for atomics
    for (int i = 0; i < cycleLength; ++i)
    {
        producedCount[i].store(0U, std::memory_order_relaxed);
        consumedCount[i].store(0U, std::memory_order_relaxed);
    }

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (int id = 0; id < numProducers; ++id)
    {
        producers.emplace_back(producePeriodic<Queue>, std::ref(queue), id, std::ref(producedCount), std::ref(run));
    }

    for (int id = 0; id < numConsumers; ++id)
    {
        consumers.emplace_back(consume<Queue>, std::ref(queue), std::ref(consumedCount), std::ref(run));
    }

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
        consumedCount[value.count].fetch_add(1U, std::memory_order_relaxed);
    }

    // verify counts
    for (int i = 0; i < cycleLength; ++i)
    {
        EXPECT_EQ(producedCount[i], consumedCount[i]);
    }
}


/// @brief Tests concurrent operation of multiple producers and consumers
///       with respect to order of the data (monotonic, FIFO).
/// @note this cannot be done easily together with completeness and limited memory
TYPED_TEST(ResizeableLockFreeQueueStressTest, multiProducerMultiConsumerOrder)
{
    using Queue = typename TestFixture::Queue;
    auto& queue = this->sut;

    std::atomic_bool run{true};

    const int numProducers{4};
    const int numConsumers{4};

    // need only one variable, any consumer that detects an error will set it to false
    // and no consumer will ever set it to true again
    std::atomic_bool orderOk{true};

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    for (int id = 0; id < numProducers; ++id)
    {
        producers.emplace_back(produceMonotonic<Queue>, std::ref(queue), id, std::ref(run));
    }

    for (int id = 0; id < numConsumers; ++id)
    {
        consumers.emplace_back(
            consumeAndCheckOrder<Queue>, std::ref(queue), numProducers - 1, std::ref(run), std::ref(orderOk));
    }

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
TYPED_TEST(ResizeableLockFreeQueueStressTest, DISABLED_hybridMultiProducerMultiConsumer)
{
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    const int numThreads = 32;

    auto capacity = q.capacity();

    // fill the queue
    Data d;
    for (size_t i = 0; i < capacity; ++i)
    {
        d.count = i;
        while (!q.tryPush(d))
            ;
    }

    std::atomic<bool> run{true};

    std::vector<std::thread> threads;

    for (int id = 1; id <= numThreads; ++id)
    {
        threads.emplace_back(work<Queue>, std::ref(q), id, std::ref(run));
    }

    std::this_thread::sleep_for(std::chrono::seconds(this->runtime));
    run = false;

    for (auto& thread : threads)
    {
        thread.join();
    }

    // check whether all elements are there, but there is no specific ordering we can expect
    std::vector<int> count(capacity, 0);
    auto popped = q.pop();
    while (popped.has_value())
    {
        count[popped.value().count]++;
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
TYPED_TEST(ResizeableLockFreeQueueStressTest, DISABLED_hybridMultiProducerMultiConsumer0verflow)
{
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    std::chrono::seconds runtime(10);
    const int numThreads = 32;
    const double popProbability = 0.45; // tends to overflow
    const auto capacity = q.capacity();

    std::atomic<bool> run{true};

    std::vector<std::thread> threads;
    std::vector<int> overflowCount(numThreads);
    std::vector<std::list<Data>> itemVec(numThreads);

    // fill the queue
    Data d;
    for (size_t i = 0; i < capacity; ++i)
    {
        d.count = i;
        while (!q.tryPush(d))
        {
        }
    }

    for (int id = 1; id <= numThreads; ++id)
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

    std::vector<std::vector<int>> count(capacity, std::vector<int>(numThreads + 1, 0));
    auto popped = q.pop();
    while (popped.has_value())
    {
        const auto& value = popped.value();
        count[value.count][value.id]++;
        popped = q.pop();
    }

    // check the local lists
    for (auto& items : itemVec)
    {
        for (auto& item : items)
        {
            count[item.count][item.id]++;
        }
    }

    bool testResult = true;
    for (size_t i = 0; i < capacity; ++i)
    {
        // we expect each data item exactly numThreads + 1 times,
        // the extra one is for the initially full queue
        // and each count appears for all ids exactly ones
        for (int j = 0; j <= numThreads; ++j)
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
TYPED_TEST(ResizeableLockFreeQueueStressTest, hybridMultiProducerMultiConsumer0verflowWithCapacityChange)
{
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    const int numThreads = 32;
    const double popProbability = 0.45; // tends to overflow
    const auto capacity = q.capacity();

    std::atomic<bool> run{true};

    std::vector<std::thread> threads;
    std::vector<int> overflowCount(numThreads);
    std::vector<std::list<Data>> itemVec(numThreads + 1);

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
    for (size_t i = 0; i < capacity; ++i)
    {
        d.count = i;
        while (!q.tryPush(d))
        {
        }
    }

    for (int id = 1; id <= numThreads; ++id)
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

    int id = numThreads + 1;
    uint64_t numChanges;
    threads.emplace_back(changeCapacity<Queue>,
                         std::ref(q),
                         std::ref(run),
                         std::ref(itemVec[id - 1]),
                         std::ref(capacities),
                         std::ref(numChanges));

    std::this_thread::sleep_for(std::chrono::seconds(this->runtime));

    run = false;

    for (auto& thread : threads)
    {
        thread.join();
    }

    // we expect at least one overflow in the test (since the queue is full in the beginning)
    // we cannot expect one overflow in each thread due to thread scheduling
    const auto numOverflows = std::accumulate(overflowCount.begin(), overflowCount.end(), 0LL);
    EXPECT_GT(numOverflows, 0LL);
    EXPECT_GT(numChanges, 0LL);

    // check whether all elements are there, but there is no specific ordering we can expect
    // items are either in the local lists or the queue, in total we expect each count numThreads times
    // and for each count each id exactly once

    std::vector<std::vector<int>> count(capacity, std::vector<int>(numThreads + 1, 0));
    auto popped = q.pop();
    while (popped.has_value())
    {
        const auto& value = popped.value();
        count[value.count][value.id]++;
        popped = q.pop();
    }

    // check the local lists
    for (auto& items : itemVec)
    {
        for (auto& item : items)
        {
            count[item.count][item.id]++;
        }
    }

    bool testResult = true;
    for (size_t i = 0; i < capacity; ++i)
    {
        // we expect each data item exactly numThreads + 1 times,
        // the extra one is for the initially full queue
        // and each count appears for all ids exactly ones
        for (int j = 0; j <= numThreads; ++j)
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
