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
#include "iox/optional.hpp"
#include "iox/signal_watcher.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-callbacks-listener-as-class-member";

class CounterService
{
  public:
    //! [ctor]
    CounterService()
        : m_subscriberLeft({"Radar", "FrontLeft", "Counter"})
        , m_subscriberRight({"Radar", "FrontRight", "Counter"})
    {
        /// Attach the static method onSampleReceivedCallback and provide this as additional argument
        /// to the callback to gain access to the object whenever the callback is called.
        /// It is not possible to use a lambda with capturing here since they are not convertable to
        /// a C function pointer.
        /// important: the user has to ensure that the contextData (*this) lives as long as
        ///            the subscriber with its callback is attached to the listener
        m_listener
            .attachEvent(m_subscriberLeft,
                         iox::popo::SubscriberEvent::DATA_RECEIVED,
                         iox::popo::createNotificationCallback(onSampleReceivedCallback, *this))
            .or_else([](auto) {
                std::cerr << "unable to attach subscriberLeft" << std::endl;
                std::exit(EXIT_FAILURE);
            });
        m_listener
            .attachEvent(m_subscriberRight,
                         iox::popo::SubscriberEvent::DATA_RECEIVED,
                         iox::popo::createNotificationCallback(onSampleReceivedCallback, *this))
            .or_else([](auto) {
                std::cerr << "unable to attach subscriberRight" << std::endl;
                std::exit(EXIT_FAILURE);
            });
    }
    //! [ctor]

  private:
    /// This method has to be static since only c functions are allowed as callback.
    /// To gain access to the members and methods of CounterClass we provide as an additional argument the this pointer
    /// which is stored in self
    //! [callback]
    static void onSampleReceivedCallback(iox::popo::Subscriber<CounterTopic>* subscriber, CounterService* self)
    {
        // take all samples from the subscriber queue
        while (subscriber->take().and_then([subscriber, self](auto& sample) {
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
        }))
        {
        }

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
    //! [callback]

    //! [members]
    iox::popo::Subscriber<CounterTopic> m_subscriberLeft;
    iox::popo::Subscriber<CounterTopic> m_subscriberRight;
    iox::optional<CounterTopic> m_leftCache;
    iox::optional<CounterTopic> m_rightCache;
    iox::popo::Listener m_listener;
    //! [members]
};

int main()
{
    //! [init]
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    CounterService counterService;

    iox::waitForTerminationRequest();
    //! [init]

    return (EXIT_SUCCESS);
}
