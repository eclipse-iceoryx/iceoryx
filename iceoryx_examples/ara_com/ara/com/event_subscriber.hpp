// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_EXAMPLES_ARA_COM_EVENT_SUBSCRIBER_HPP
#define IOX_EXAMPLES_ARA_COM_EVENT_SUBSCRIBER_HPP

#include "iceoryx_posh/popo/subscriber.hpp"

#include "ara/types.hpp"

#include <limits>
#include <memory>

namespace ara
{
namespace com
{
template <typename T>
class EventSubscriber
{
  public:
    using SampleType = T;

    EventSubscriber(const core::String& service, const core::String& instance, const core::String& event) noexcept
        : m_subscriber({service, instance, event})
    {
    }

    void Subscribe(std::size_t maxSampleCount) noexcept
    {
        m_subscriber.subscribe(maxSampleCount);
    }

    void Unsubscribe() noexcept
    {
        m_subscriber.unsubscribe();
    }

    template <typename Callable>
    ara::core::Result<size_t> GetNewSamples(Callable&& callable,
                                            size_t maxNumberOfSamples = std::numeric_limits<size_t>::max()) noexcept
    {
        IOX_DISCARD_RESULT(maxNumberOfSamples);

        ara::core::Result<size_t> numberOfSamples{0};

        while (m_subscriber.take()
                   .and_then([&](const auto& sample) {
                       callable(sample.get());
                       numberOfSamples++;
                   })
                   .or_else([](auto& result) {
                       if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                       {
                           std::cout << "Error receiving chunk!" << std::endl;
                       }
                   }))
        {
        }
        return numberOfSamples;
    }

  private:
    iox::popo::Subscriber<T> m_subscriber;
};

} // namespace com
} // namespace ara

#endif // IOX_EXAMPLES_ARA_COM_EVENT_SUBSCRIBER_HPP
