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

#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "test.hpp"
#include <atomic>

namespace
{
using namespace ::testing;
using namespace iox::posix;

class SignalWatcherTester : public SignalWatcher
{
  public:
    SignalWatcherTester() = default;
};

class SignalWatcher_test : public Test
{
  public:
    void SetUp() override
    {
        sut = static_cast<SignalWatcherTester*>(&SignalWatcher::getInstance());
        sut->~SignalWatcherTester();
        new (sut) SignalWatcherTester();
        watchdog.watchAndActOnFailure([] { std::terminate(); });
    }

    void TearDown() override
    {
    }

    SignalWatcherTester* sut;
    Watchdog watchdog{iox::units::Duration::fromSeconds(2)};
    std::chrono::milliseconds waitingTime = std::chrono::milliseconds(10);
};

TEST_F(SignalWatcher_test, SignalWasNotTriggeredWhenNotTriggeredBefore)
{
    EXPECT_FALSE(sut->wasSignalTriggered());
    EXPECT_FALSE(hasTerminationRequest());
}

TEST_F(SignalWatcher_test, SignalIsTriggeredWhenSIGINTWasTriggeredBefore)
{
    raise(SIGINT);
    EXPECT_TRUE(sut->wasSignalTriggered());
    EXPECT_TRUE(hasTerminationRequest());
}

TEST_F(SignalWatcher_test, SignalIsTriggeredWhenSIGTERMWasTriggeredBefore)
{
    raise(SIGTERM);
    EXPECT_TRUE(sut->wasSignalTriggered());
    EXPECT_TRUE(hasTerminationRequest());
}

void unblocksWhenSignalWasRaisedForWaiters(SignalWatcher_test& test,
                                           const int signal,
                                           const uint64_t numberOfWaiters,
                                           const std::function<void()>& wait)
{
    std::atomic<uint64_t> isThreadStarted{0};
    std::atomic<uint64_t> isThreadFinished{0};

    std::vector<std::thread> threads;

    for (uint64_t i = 0; i < numberOfWaiters; ++i)
    {
        threads.emplace_back([&] {
            ++isThreadStarted;
            wait();
            ++isThreadFinished;
        });
    }

    while (isThreadStarted.load() != numberOfWaiters)
    {
        std::this_thread::yield();
    }

    std::this_thread::sleep_for(test.waitingTime);

    EXPECT_TRUE(isThreadFinished.load() == 0);
    raise(signal);

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
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGINT, 1, [&] { sut->waitForSignal(); });
}

TEST_F(SignalWatcher_test, UnblocksWhenSIGTERMWasRaisedForOneWaiter)
{
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGTERM, 1, [&] { sut->waitForSignal(); });
}


TEST_F(SignalWatcher_test, UnblocksWhenSIGINTWasRaisedForMultipleWaiter)
{
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGINT, 3, [&] { sut->waitForSignal(); });
}

TEST_F(SignalWatcher_test, UnblocksWhenSIGTERMWasRaisedForMultipleWaiter)
{
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGTERM, 4, [&] { sut->waitForSignal(); });
}

TEST_F(SignalWatcher_test, UnblocksWhenSIGINTWasRaisedForOneWaiterWithConvenienceFunction)
{
    unblocksWhenSignalWasRaisedForWaiters(*this, SIGINT, 1, [&] { waitForTerminationRequest(); });
}


} // namespace
