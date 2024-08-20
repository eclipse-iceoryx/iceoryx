// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iox/atomic.hpp"
#include "iox/signal_watcher.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;

class SignalWatcherTester : public SignalWatcher
{
  public:
    SignalWatcherTester() = default;
    static void reset() noexcept
    {
        SignalWatcher* currentInstance = &getInstance();
        currentInstance->~SignalWatcher();
        new (currentInstance) SignalWatcherTester();
    }
};

class SignalWatcher_test : public Test
{
  public:
    void SetUp() override
    {
        SignalWatcherTester::reset();
        sut = &SignalWatcher::getInstance();
        watchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    void TearDown() override
    {
        SignalWatcherTester::reset();
    }

    SignalWatcher* sut = nullptr;
    Watchdog watchdog{iox::units::Duration::fromSeconds(2)};
    std::chrono::milliseconds waitingTime = std::chrono::milliseconds(10);
};

TEST_F(SignalWatcher_test, SignalWasNotTriggeredWhenNotTriggeredBefore)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe493293-b64c-4f4f-a630-ea17cb5365da");
    EXPECT_FALSE(sut->wasSignalTriggered());
    EXPECT_FALSE(hasTerminationRequested());
}

TEST_F(SignalWatcher_test, SignalIsTriggeredWhenSIGINTWasTriggeredBefore)
{
    ::testing::Test::RecordProperty("TEST_ID", "48e18aae-af21-43c4-a444-70fc371d328f");
    ASSERT_EQ(raise(SIGINT), 0);
    EXPECT_TRUE(sut->wasSignalTriggered());
    EXPECT_TRUE(hasTerminationRequested());
}

TEST_F(SignalWatcher_test, SignalIsTriggeredWhenSIGTERMWasTriggeredBefore)
{
    ::testing::Test::RecordProperty("TEST_ID", "639708fa-3327-4573-92e2-cdbbff2cbdec");
    ASSERT_EQ(raise(SIGTERM), 0);
    EXPECT_TRUE(sut->wasSignalTriggered());
    EXPECT_TRUE(hasTerminationRequested());
}

void unblocksWhenSignalWasRaisedForWaiters(SignalWatcher_test& test,
                                           // NOLINTJUSTIFICATION only used inside this test
                                           // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                           const int signal,
                                           const uint32_t numberOfWaiters,
                                           const std::function<void()>& wait)
{
    Barrier isThreadStarted(numberOfWaiters);
    iox::concurrent::Atomic<uint64_t> isThreadFinished{0};

    std::vector<std::thread> threads;

    for (uint64_t i = 0; i < numberOfWaiters; ++i)
    {
        threads.emplace_back([&] {
            isThreadStarted.notify();
            wait();
            ++isThreadFinished;
        });
    }

    isThreadStarted.wait();

    std::this_thread::sleep_for(test.waitingTime);

    EXPECT_TRUE(isThreadFinished.load() == 0);
    ASSERT_EQ(raise(signal), 0);

    while (isThreadFinished.load() != numberOfWaiters)
    {
        std::this_thread::yield();
    }

    for (auto& t : threads)
    {
        t.join();
    }
}

TEST_F(SignalWatcher_test, UnblocksWhenSIGINTWasRaisedForOneWaiter)
{
    ::testing::Test::RecordProperty("TEST_ID", "52812e86-b6e8-4d04-9279-f5c5ecc04d35");
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGINT, 1, [&] { sut->waitForSignal(); });
}

TEST_F(SignalWatcher_test, UnblocksWhenSIGTERMWasRaisedForOneWaiter)
{
    ::testing::Test::RecordProperty("TEST_ID", "f5ffc62f-3ce8-4835-8ae4-1805dda2aa59");
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGTERM, 1, [&] { sut->waitForSignal(); });
}


TEST_F(SignalWatcher_test, UnblocksWhenSIGINTWasRaisedForMultipleWaiter)
{
    ::testing::Test::RecordProperty("TEST_ID", "b63d4450-3a69-499f-b1f5-5c64360a259b");
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGINT, 3, [&] { sut->waitForSignal(); });
}

TEST_F(SignalWatcher_test, UnblocksWhenSIGTERMWasRaisedForMultipleWaiter)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a46cbc6-5a72-4dd3-a60f-d90e7f10b849");
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGTERM, 4, [&] { sut->waitForSignal(); });
}

TEST_F(SignalWatcher_test, UnblocksWhenSIGINTWasRaisedForOneWaiterWithConvenienceFunction)
{
    ::testing::Test::RecordProperty("TEST_ID", "b051206b-15a0-46eb-9566-325bb59830ca");
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGINT, 1, [&] { waitForTerminationRequest(); });
}


} // namespace
