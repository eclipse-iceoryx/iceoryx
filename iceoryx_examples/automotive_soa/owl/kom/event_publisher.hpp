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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_PUBLISHER_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_PUBLISHER_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/publisher.hpp"

#include "owl/types.hpp"

#include <memory>
#include <utility>

class MinimalSkeleton;

namespace owl
{
namespace kom
{
template <typename T>
class EventPublisher
{
  public:
    using SampleType = T;

    // Deleted because of SampleAllocateePtr implementation capturing 'this' in Allocate()
    EventPublisher(const EventPublisher&) = delete;
    EventPublisher(EventPublisher&&) = delete;
    EventPublisher& operator=(const EventPublisher&) = delete;
    EventPublisher& operator=(EventPublisher&&) = delete;

    EventPublisher(const core::String& service, const core::String& instance, const core::String& event) noexcept
        : m_publisher({service, instance, event}, {1U, "", false})
    {
    }

    void Send(const SampleType& userSample) noexcept
    {
        auto maybeSample = m_publisher.loan();

        if (maybeSample.has_error())
        {
            std::cout << "Error occured during allocation, couldn't send sample!" << std::endl;
            return;
        }

        auto& sample = maybeSample.value();
        *(sample.get()) = userSample;
        sample.publish();
    }

    owl::kom::SampleAllocateePtr<SampleType> Allocate() noexcept
    {
        auto maybeSample = m_publisher.loan();

        if (maybeSample.has_error())
        {
            return iox::cxx::nullopt;
        }

        return owl::kom::SampleAllocateePtr<SampleType>(std::move(maybeSample.value()));
    }

    void Send(owl::kom::SampleAllocateePtr<SampleType> userSamplePtr) noexcept
    {
        /// @todo #1332 how to verify that this sample belongs to me?
        userSamplePtr.value().publish();
    }

    friend class ::MinimalSkeleton;

  private:
    void Offer() noexcept
    {
        m_publisher.offer();
    }

    void StopOffer() noexcept
    {
        m_publisher.stopOffer();
    }

    iox::popo::Publisher<T> m_publisher;
};
} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_EVENT_PUBLISHER_HPP
