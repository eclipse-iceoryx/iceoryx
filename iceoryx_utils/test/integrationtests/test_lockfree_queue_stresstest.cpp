// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_utils/concurrent/lockfree_queue.hpp"
using namespace ::testing;

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
    Data(int id = 0, size_t count = 0)
        : id(id)
        , count(count)
    {
    }

    int id{0};
    size_t count{0};

    void print()
    {
        std::cout << "data id " << id << " count " << count << std::endl;
    }
};

template <typename Queue>
void produce(Queue& queue, int id, int iterations)
{
    Data d(id, 0);
    for (int i = 0; i < iterations; ++i)
    {
        d.count++;
        while (!queue.tryPush(d))
        {
        }
    }
}

template <typename Queue>
void consume(Queue& queue, std::atomic<bool>& run, size_t expectedFinalCount, int maxId, bool& testResult)
{
    bool error = false;

    std::vector<size_t> lastCount(maxId + 1, 0);

    while (run || !queue.empty())
    {
        auto popped = queue.pop();
        if (popped.has_value())
        {
            auto& value = popped.value();
            if (lastCount[value.id] + 1 != value.count)
            {
                error = true;
            }

            lastCount[value.id] = value.count;
        }
    }

    for (int i = 1; i <= maxId; ++i)
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
void consumeAndStore(Queue& queue, std::atomic<bool>& run, std::list<Data>& consumed)
{
    while (run || !queue.empty())
    {
        auto popped = queue.pop();
        if (popped.has_value())
        {
            consumed.push_back(popped.value());
        }
    }
}

std::list<Data> filter(std::list<Data>& list, int id)
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

bool isStrictlyMonotonous(std::list<Data>& list)
{
    auto iter = list.begin();
    if (iter == list.end())
    {
        return true;
    }

    size_t prev = iter->count;
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
        count[data.count]++;
    }

    for (auto& data : list2)
    {
        count[data.count]++;
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

bool checkTwoConsumerResult(std::list<Data>& consumed1,
                            std::list<Data>& consumed2,
                            size_t expectedFinalCount,
                            int maxId)
{
    std::vector<std::list<Data>> consumed(maxId + 1);

    for (int id = 1; id <= maxId; ++id)
    {
        auto filtered1 = filter(consumed1, id);
        auto filtered2 = filter(consumed2, id);

        if (!isStrictlyMonotonous(filtered1) || !isStrictlyMonotonous(filtered2))
        {
            std::cout << "id " << id << " not strictly monotonous" << std::endl;
            return false;
        }

        if (!isComplete(filtered1, filtered2, expectedFinalCount))
        {
            std::cout << "id " << id << " incomplete" << std::endl;
            return false;
        }
    }

    return true;
}


// alternates between push and pop
template <typename Queue>
void work(Queue& queue, int id, std::atomic<bool>& run)
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
void randomWork(Queue& queue,
                int id,
                std::atomic<bool>& run,
                int& overflowCount,
                std::list<Data>& items,
                double popProbability = 0.5)
{
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
        doPop = (dist(rng) <= popProbability) ? true : false;
    }
}


template <typename T>
class LockFreeQueueStressTest : public ::testing::Test
{
  protected:
    LockFreeQueueStressTest() = default;

    ~LockFreeQueueStressTest()
    {
    }

    void SetUp()
    {
        // internal::CaptureStdout();
    }

    void TearDown()
    {
        if (Test::HasFailure())
        {
            // std::string output = internal::GetCapturedStdout();
            // std::cout << output << std::endl;
        }
    }

    using Queue = T;
    Queue sut;
};

template <size_t Capacity>
using TestQueue = iox::concurrent::LockFreeQueue<Data, Capacity>;
using SingleElementQueue = TestQueue<1>;
using SmallQueue = TestQueue<10>;
using MediumQueue = TestQueue<1000>;
using LargeQueue = TestQueue<1000000>;

// each of the following tests is run for different queue sizes specified here

typedef ::testing::Types<SingleElementQueue, SmallQueue, MediumQueue, LargeQueue> TestQueues;
// typedef ::testing::Types<MediumQueue> TestQueues;

/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(LockFreeQueueStressTest, TestQueues);
#pragma GCC diagnostic pop

///@brief Tests concurrent operation of one prodcuer and one consumer
/// The producer pushes a fixed number of data elements which the consumer pops and checks.
/// The order of popped elements and completeness (no data loss) is checked.
TYPED_TEST(LockFreeQueueStressTest, DISABLED_singleProducerSingleConsumer)
{
    using Queue = typename TestFixture::Queue;

    auto& queue = this->sut;
    std::atomic<bool> run{true};
    bool testResult;
    int iterations = 10000000;

    std::thread consumer(consume<Queue>, std::ref(queue), std::ref(run), iterations, 1, std::ref(testResult));
    std::thread producer(produce<Queue>, std::ref(queue), 1, iterations);

    producer.join();

    run = false;
    consumer.join();

    EXPECT_EQ(testResult, true);
}

///@brief Tests concurrent operation of multiple prodcuers and one consumer.
/// The producers push a fixed number of data elements which the consumer pops and checks.
/// The order of popped elements and completeness (no data loss) is checked.
TYPED_TEST(LockFreeQueueStressTest, DISABLED_multiProducerSingleConsumer)
{
    using Queue = typename TestFixture::Queue;

    auto& queue = this->sut;
    std::atomic<bool> run{true};
    bool testResult;
    int iterations = 1000000;
    int numProducers = 8;

    std::vector<std::thread> producers;

    std::thread consumer(
        consume<Queue>, std::ref(queue), std::ref(run), iterations, numProducers, std::ref(testResult));

    for (int id = 1; id <= numProducers; ++id)
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
TYPED_TEST(LockFreeQueueStressTest, DISABLED_multiProducerTwoConsumer)
{
    using Queue = typename TestFixture::Queue;

    auto& queue = this->sut;
    std::atomic<bool> run{true};
    int iterations = 1000000;
    int numProducers = 4;

    std::vector<std::thread> producers;

    // the lists will get quite large  (in total they hold all produced elements,
    // i.e. iterations * numProducers
    std::list<Data> consumed1;
    std::list<Data> consumed2;
    std::thread consumer1(consumeAndStore<Queue>, std::ref(queue), std::ref(run), std::ref(consumed1));
    std::thread consumer2(consumeAndStore<Queue>, std::ref(queue), std::ref(run), std::ref(consumed2));

    for (int id = 1; id <= numProducers; ++id)
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
TYPED_TEST(LockFreeQueueStressTest, DISABLED_timedMultiProducerMultiConsumer)
{
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    std::chrono::seconds runtime(10);
    int numThreads = 32;

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

    std::this_thread::sleep_for(std::chrono::seconds(runtime));

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
TYPED_TEST(LockFreeQueueStressTest, DISABLED_timedMultiProducerMultiConsumer0verflow)
{
    using Queue = typename TestFixture::Queue;

    auto& q = this->sut;
    std::chrono::seconds runtime(10);
    int numThreads = 32;
    double popProbability = 0.45; // tends to overflow

    auto capacity = q.capacity();

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

    // check whether all elements are there, but there is no specific ordering we can expect
    // items are either in the local lists or the queue, in total we expect each count numThreads times

    std::vector<int> count(capacity, 0);
    auto popped = q.pop();
    while (popped.has_value())
    {
        count[popped.value().count]++;
        popped = q.pop();
    }

    // check the local lists
    for (auto& items : itemVec)
    {
        for (auto& item : items)
        {
            count[item.count]++;
        }
    }

    // we expect at least one overflow in the test (since the queue is full in the beginning)
    // we cannot expect one overflow in each thread due to thread scheduling
    auto numOverflows = std::accumulate(overflowCount.begin(), overflowCount.end(), 0LL);
    EXPECT_GT(numOverflows, 0LL);

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
