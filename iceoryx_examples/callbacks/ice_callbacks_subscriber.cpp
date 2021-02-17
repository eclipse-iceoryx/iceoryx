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

#include "iceoryx_posh/popo/active_call_set.hpp"
#include "iceoryx_posh/popo/typed_subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

iox::popo::UserTrigger shutdownTrigger;
iox::posix::Semaphore mainLoopBlocker =
    iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0).value();

std::atomic_bool keepRunning{true};

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownTrigger.trigger();
}

void shutdownTriggerCallback(iox::popo::UserTrigger*)
{
    keepRunning.store(false);
}

void subscriberCallback(iox::popo::TypedSubscriber<CounterTopic>* subscriber)
{
    subscriber->take_1_0().and_then([](auto& sample) { printf("received: %d\n", sample->counter); });
}

int main()
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = sigHandler;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, nullptr);

    iox::runtime::PoshRuntime::initRuntime("iox-ex-callbacks-subscriber");

    iox::popo::ActiveCallSet callSet;

    // attach shutdownTrigger to handle CTRL+C
    callSet.attachEvent(shutdownTrigger, shutdownTriggerCallback);

    iox::popo::TypedSubscriber<CounterTopic> subscriber({"Radar", "FrontLeft", "Counter"});

    subscriber.subscribe();

    callSet.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLES, subscriberCallback);

    while (keepRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    callSet.detachEvent(shutdownTrigger);
    callSet.detachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLES);

    return (EXIT_SUCCESS);
}
