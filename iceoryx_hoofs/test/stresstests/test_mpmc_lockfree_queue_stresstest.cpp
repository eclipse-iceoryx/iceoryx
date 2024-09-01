// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 Apex.AI Inc. All rights reserved.
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
#include "iox/logging.hpp"

#include "iox/detail/mpmc_lockfree_queue.hpp"
using namespace ::testing;

#include "iceoryx_hoofs/testing/barrier.hpp"

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
    explicit Data(uint64_t id = 0U, uint64_t count = 0)
        : id(id)
        , count(count)
    {
    }

    uint64_t id{0};
    uint64_t count{0};

    void print() const
    {
        IOX_LOG(INFO, "data id " << id << " count " << count);
    }
};

Barrier g_barrier;

template <typename Queue>
//NOLINTNEXTLINE(bugprone-easily-swappable-parameters) This is okay since it is limited to the stress test
void produce(Queue& queue, uint64_t id, uint64_t iterations)
{
    g_barrier.notify();

    Data d(id, 0);
    for (uint64_t i = 0; i < iterations; ++i)
    {
        d.count++;
        while (!queue.tryPush(d))
        {
        }
    }
}

template <typename Queue>
//NOLINTBEGIN(bugprone-easily-swappable-parameters, readability-function-size) This is okay since it is limited to the stress test
void consume(
    Queue& queue, iox::concurrent::Atomic<bool>& run, uint64_t expectedFinalCount, uint64_t maxId, bool& testResult)
//NOLINTEND(bugprone-easily-swappable-parameters, readability-function-size)
{
    g_barrier.notify();

    bool error = false;

    std::vector<uint64_t> lastCount(static_cast<size_t>(maxId) + 1U, 0);

    while (run || !queue.empty())
    {
        auto popped = queue.pop();
        if (popped.has_value())
        {
            auto& value = popped.value();
            if (lastCount[static_cast<size_t>(value.id)] + 1 != value.count)
            {
                error = true;
            }

            lastCount[static_cast<size_t>(value.id)] = value.count;
        }
    }

    for (size_t i = 1; i <= maxId; ++i)
    {
        if (lastCount[i] != expectedFinalCount)
        {
            error = true;
        }
    }

    testResult = !error;
}


/// remark: a possible rework could try to avoid storing the popped values for check with multiple consumers
/// since this would allow us to run the test much longer (currently we will exhaust memory
/// by using the list), but this rework is somewhat nontrivial
template <typename Queue>
void consumeAndStore(Queue& queue, iox::concurrent::Atomic<bool>& run, std::list<Data>& consumed)
{
    g_barrier.notify();

    while (run || !queue.empty())
    {
        auto popped = queue.pop();
        if (popped.has_value())
        {
            consumed.push_back(popped.value());
        }
    }
}

std::list<Data> filter(std::list<Data>& list, uint64_t id)
{
    std::list<Data> filtered;

    for (auto& data : list)
    {
        if (data.id == id)
        {
            filtered.push_back(data);
        }
    }

    return filtered;
}

bool isStrictlyMonotonic(std::list<Data>& list)
{
    auto iter = list.begin();
    if (iter == list.end())
    {
        return true;
    }

    auto prev = iter->count;
    iter++;

    while (iter != list.end())
    {
        if (prev >= iter->count)
        {
            return false;
        }
        prev = iter->count;
        iter++;
    }

    return true;
}

bool isComplete(std::list<Data>& list1, std::list<Data>& list2, size_t finalCount)
{
    std::vector<int> count(finalCount + 1);
    for (auto& data : list1)
    {
        count[static_cast<size_t>(data.count)]++;
    }

    for (auto& data : list2)
    {
        count[static_cast<size_t>(data.count)]++;
    }

    for (size_t i = 1; i <= finalCount; ++i)
    {
        if (count[i] != 1)
        {
            return false;
        }
    }

    return true;
}

//NOLINTBEGIN(bugprone-easily-swappable-parameters) This is okay since it is limited to the stress test
bool checkTwoConsumerResult(std::list<Data>& consumed1,
                            std::list<Data>& consumed2,
                            uint64_t expectedFinalCount,
                            uint64_t maxId)
{
    std::vector<std::list<Data>> consumed(static_cast<size_t>(maxId) + 1U);

    for (uint64_t id = 1; id <= maxId; ++id)
    {
        auto filtered1 = filter(consumed1, id);
        auto filtered2 = filter(consumed2, id);

        if (!isStrictlyMonotonic(filtered1) || !isStrictlyMonotonic(filtered2))
        {
            IOX_LOG(INFO, "id " << id << " not strictly monotonic");
            return false;
        }

        if (!isComplete(filtered1, filtered2, static_cast<size_t>(expectedFinalCount)))
        {
            IOX_LOG(INFO, "id " << id << " incomplete");
            return false;
        }
    }

    return true;
}
//NOLINTEND(bugprone-easily-swappable-parameters)

// alternates between push and pop
template <typename Queue>
void work(Queue& queue, uint64_t id, iox::concurrent::Atomic<bool>& run)
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
            value.id = id;
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
                uint64_t id,
                iox::concurrent::Atomic<bool>& run,
                uint64_t& overflowCount,
                std::list<Data>& items,
                double popProbability = 0.5)
{
    g_barrier.notify();

    Data value;
    value.id = id;

    auto capacity = queue.capacity();

    // populate the list with capacity unique items (with this workers id)
    for (value.count = 0; value.count < capacity; ++value.count)
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


template <typename T>
class MpmcLockFreeQueueStressTest : public ::testing::Test
{
  protected:
    MpmcLockFreeQueueStressTest() = default;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    using Queue = T;
    Queue sut;
};

template <size_t Capacity>
using TestQueue = iox::concurrent::MpmcLockFreeQueue<Data, Capacity>;
using SingleElementQueue = TestQueue<1>;
using SmallQueue = TestQueue<10>;
using MediumQueue = TestQueue<1000>;
using LargeQueue = TestQueue<1000000>;

// each of the following tests is run for different queue sizes specified here

typedef ::testing::Types<SingleElementQueue, SmallQueue, MediumQueue, LargeQueue> TestQueues;
// typedef ::testing::Types<MediumQueue> TestQueues;

TYPED_TEST_SUITE(MpmcLockFreeQueueStressTest, TestQueues, );

///@brief Tests concurrent operation of one producer and one consumer
/// The producer pushes a fixed number of data elements which the consumer pops and checks.
/// The order of popped elements and completeness (no data loss) is checked.
TYPED_TEST(MpmcLockFreeQueueStressTest, SingleProducerSingleConsumer)
{
    ::testing::Test::RecordProperty("TEST_ID", "6987ee43-aeac-48dc-ba91-b9cc1f479d9b");
    using Queue = typename TestFixture::Queue;

    auto& queue = this->sut;
    iox::concurrent::Atomic<bool> run{true};
    bool testResult{false};
    int iterations = 10000000;

    std::thread consumer(consume<Queue>, std::ref(queue), std::ref(run), iterations, 1U, std::ref(testResult));
    std::thread producer(produce<Queue>, std::ref(queue), 1U, iterations);

    producer.join();

    run = false;
    consumer.join();

    EXPECT_EQ(testResult, true);
}

///@brief Tests concurrent operation of multiple producers and one consumer.
/// The producers push a fixed number of data elements which the consumer pops and checks.
/// The order of popped elements and completeness (no data loss) is checked.
TYPED_TEST(MpmcLockFreeQueueStressTest, MultiProducerSingleConsumer)
{
    ::testing::Test::RecordProperty("TEST_ID", "e89cad23-5c2a-4da7-8d02-557aa3058e65");
    using Queue = typename TestFixture::Queue;

    auto& queue = this->sut;
    iox::concurrent::Atomic<bool> run{true};
    bool testResult{false};
    uint64_t iterations = 1000000U;
    uint64_t numProducers = 8U;

    std::vector<std::thread> producers;

    std::thread consumer(
        consume<Queue>, std::ref(queue), std::ref(run), iterations, numProducers, std::ref(testResult));

    for (uint64_t id = 1U; id <= numProducers; ++id)
    {
        producers.emplace_back(produce<Queue>, std::ref(queue), id, iterations);
    }

    for (auto& producer : producers)
    {
        producer.join();
    }

    run = false;
    consumer.join();

    EXPECT_EQ(testResult, true);
}

///@brief Tests concurrent operation of multiple producers and two consumers.
/// The producers push a fixed number of data elements which the consumers pop and store for checks
/// after the threads finish their operation.
/// The order of popped elements and completeness (no data loss) is checked.
/// Note: it gets complicated if we want to check for data completeness (i.e. no data loss)
/// with multiple consumers > 2, so for now we just do this for 2 consumers.
/// This is especially the case for many producers/iterations since we need to store intermediate
/// states from the consumers to check later (which can get quite large).
TYPED_TEST(MpmcLockFreeQueueStressTest, MultiProducerTwoConsumer)
{
    ::testing::Test::RecordProperty("TEST_ID", "73d6047a-50b3-4d1d-9e64-a99ed3d393d1");
    using Queue = typename TestFixture::Queue;

    auto& queue = this->sut;
    iox::concurrent::Atomic<bool> run{true};
    uint64_t iterations = 1000000U;
    uint64_t numProducers = 4;

    std::vector<std::thread> producers;

    // the lists will get quite large  (in total they hold all produced elements,
    // i.e. iterations * numProducers
    std::list<Data> consumed1;
    std::list<Data> consumed2;
    std::thread consumer1(consumeAndStore<Queue>, std::ref(queue), std::ref(run), std::ref(consumed1));
    std::thread consumer2(consumeAndStore<Queue>, std::ref(queue), std::ref(run), std::ref(consumed2));

    for (uint64_t id = 1U; id <= numProducers; ++id)
    {
        producers.emplace_back(produce<Queue>, std::ref(queue), id, iterations);
    }

    for (auto& producer : producers)
    {
        producer.join();
    }
    run = false;
    consumer1.join();
    consumer2.join();

    EXPECT_EQ(checkTwoConsumerResult(consumed1, consumed2, iterations, numProducers), true);
}


///@brief Tests concurrent operation of multiple hybrid producer/consumer threads.
/// The tests initializes a queue full of distinct (unique) elements
/// and each thread alternates between pop and push, only pushing what he has previously popped.
/// The test runs for some specified time and upon completion each thread pushes every consumed
/// data item back into the queue.
/// Finally it is checked whether the queue still contains all elements it was initialized with
///(likely in a different order).
TYPED_TEST(MpmcLockFreeQueueStressTest, TimedMultiProducerMultiConsumer)
{
    ::testing::Test::RecordProperty("TEST_ID", "9e18cc73-a5e8-4874-aa75-09d170477d36");
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    std::chrono::seconds runtime(10);
    uint32_t numThreads = 32U;

    auto capacity = q.capacity();

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

    iox::concurrent::Atomic<bool> run{true};

    std::vector<std::thread> threads;

    for (uint64_t id = 1; id <= numThreads; ++id)
    {
        threads.emplace_back(work<Queue>, std::ref(q), id, std::ref(run));
    }

    g_barrier.wait();
    std::this_thread::sleep_for(std::chrono::seconds(runtime));

    run = false;

    for (auto& thread : threads)
    {
        thread.join();
    }

    // check whether all elements are there, but there is no specific ordering we can expect
    std::vector<int> count(static_cast<size_t>(capacity), 0);
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
/// Each thread chooses randomly between push and pop (preference is controllable, to make overflow more or less
/// likely).
/// The test runs for some specified time and upon completion it is checked that
/// aggregated over the queue and the local lists of each thread
/// all elements occur exactly as often as there are threads + 1 (i.e. nothing was lost, the +1 is
/// due to the initial values in the queue itself).
TYPED_TEST(MpmcLockFreeQueueStressTest, TimedMultiProducerMultiConsumer0verflow)
{
    ::testing::Test::RecordProperty("TEST_ID", "9adf76d8-beeb-4810-ac71-c4c9dd363219");
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    std::chrono::seconds runtime(10);
    uint32_t numThreads = 32U;
    double popProbability = 0.45; // tends to overflow

    auto capacity = q.capacity();

    iox::concurrent::Atomic<bool> run{true};

    std::vector<std::thread> threads;
    std::vector<uint64_t> overflowCount(numThreads);
    std::vector<std::list<Data>> itemVec(numThreads);

    g_barrier.reset(numThreads);

    // fill the queue
    Data d;
    for (uint64_t i = 0U; i < capacity; ++i)
    {
        d.count = i;
        while (!q.tryPush(d))
        {
        }
    }

    for (uint64_t id = 1U; id <= numThreads; ++id)
    {
        threads.emplace_back(randomWork<Queue>,
                             std::ref(q),
                             id,
                             std::ref(run),
                             std::ref(overflowCount[static_cast<size_t>(id) - 1]),
                             std::ref(itemVec[static_cast<size_t>(id) - 1]),
                             popProbability);
    }

    g_barrier.wait();
    std::this_thread::sleep_for(std::chrono::seconds(runtime));

    run = false;

    for (auto& thread : threads)
    {
        thread.join();
    }

    // check whether all elements are there, but there is no specific ordering we can expect
    // items are either in the local lists or the queue, in total we expect each count numThreads times

    std::vector<uint64_t> count(static_cast<size_t>(capacity), 0U);
    auto popped = q.pop();
    while (popped.has_value())
    {
        count[static_cast<size_t>(popped.value().count)]++;
        popped = q.pop();
    }

    // check the local lists
    for (auto& items : itemVec)
    {
        for (auto& item : items)
        {
            count[static_cast<size_t>(item.count)]++;
        }
    }

    // we expect at least one overflow in the test (since the queue is full in the beginning)
    // we cannot expect one overflow in each thread due to thread scheduling
    auto numOverflows = std::accumulate(overflowCount.begin(), overflowCount.end(), 0ULL);
    EXPECT_GT(numOverflows, 0ULL);

    bool testResult = true;
    for (size_t i = 0; i < capacity; ++i)
    {
        // we expect each data item exactly numThreads + 1 times,
        // the extra one is for the initially full queue
        if (count[i] != numThreads + 1)
        {
            testResult = false;
            break;
        }
    }

    EXPECT_EQ(testResult, true);
}

} // namespace
