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

#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

std::atomic_bool keepRunning{true};
constexpr char APP_NAME[] = "iox-cpp-callbacks-subscriber";

iox::posix::Semaphore shutdownSemaphore =
    iox::posix::Semaphore::create(iox::posix::CreateUnnamedSingleProcessSemaphore, 0U).value();

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownSemaphore.post().or_else([](auto) {
        std::cerr << "unable to call post on shutdownSemaphore - semaphore corrupt?" << std::endl;
        std::terminate();
    });
    keepRunning = false;
}

class CounterClass
{
  public:
    CounterClass()
        : m_subscriberLeft({"Radar", "FrontLeft", "Counter"})
        , m_subscriberRight({"Radar", "FrontRight", "Counter"})
    {
        m_listener
            .attachEvent(m_subscriberLeft, iox::popo::SubscriberEvent::DATA_RECEIVED, onSampleReceivedCallback, *this)
            .or_else([](auto) {
                std::cerr << "unable to attach subscriberLeft" << std::endl;
                std::terminate();
            });
        // it is possible to attach any callback here with the required signature. to simplify the
        // example we attach the same callback onSampleReceivedCallback again
        m_listener
            .attachEvent(m_subscriberRight, iox::popo::SubscriberEvent::DATA_RECEIVED, onSampleReceivedCallback, *this)
            .or_else([](auto) {
                std::cerr << "unable to attach subscriberRight" << std::endl;
                std::terminate();
            });
    }

    void waitForControlC() noexcept
    {
        shutdownSemaphore.wait().or_else(
            [](auto) { std::cerr << "unable to call wait on shutdownSemaphore - semaphore corrupt?" << std::endl; });
    }

  private:
    static void onSampleReceivedCallback(iox::popo::Subscriber<CounterTopic>* subscriber, CounterClass* self)
    {
        subscriber->take().and_then([subscriber, self](auto& sample) {
            auto instanceString = subscriber->getServiceDescription().getInstanceIDString();

            // store the sample in the corresponding cache
            if (instanceString == iox::capro::IdString_t("FrontLeft"))
            {
                self->m_leftCache.emplace(*sample);
            }
            else if (instanceString == iox::capro::IdString_t("FrontRight"))
            {
                self->m_rightCache.emplace(*sample);
            }

            std::cout << "received: " << sample->counter << std::endl;
        });

        // if both caches are filled we can process them
        if (self->m_leftCache && self->m_rightCache)
        {
            std::cout << "Received samples from FrontLeft and FrontRight. Sum of " << self->m_leftCache->counter
                      << " + " << self->m_rightCache->counter << " = "
                      << self->m_leftCache->counter + self->m_rightCache->counter << std::endl;
            self->m_leftCache.reset();
            self->m_rightCache.reset();
        }
    }

    iox::popo::Listener m_listener;
    iox::popo::Subscriber<CounterTopic> m_subscriberLeft;
    iox::popo::Subscriber<CounterTopic> m_subscriberRight;
    iox::cxx::optional<CounterTopic> m_leftCache;
    iox::cxx::optional<CounterTopic> m_rightCache;
};

int main()
{
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    CounterClass counterClass;

    counterClass.waitForControlC();

    return (EXIT_SUCCESS);
}
