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

#include "topic.hpp"

#include "owl/kom/event_publisher.hpp"
#include "owl/kom/field_publisher.hpp"
#include "owl/kom/method_server.hpp"
#include "owl/types.hpp"

class MinimalSkeleton
{
  public:
    /// @todo make c'tor of cxx::string constexpr'able
    static constexpr char m_serviceIdentifier[] = "MinimalSkeleton";

    MinimalSkeleton(owl::core::String& instanceIdentifier)
        : m_instanceIdentifier(instanceIdentifier)
    {
    }
    MinimalSkeleton(const MinimalSkeleton&) = delete;
    MinimalSkeleton& operator=(const MinimalSkeleton&) = delete;

    void OfferService() noexcept
    {
        /// @todo Should call PoshRuntime::OfferService, how to add the Service Registry entry instead?
        m_event.Offer();
    }

    void StopOfferService() noexcept
    {
        /// @todo Should call PoshRuntime::StopOfferService, how to remove the Service Registry entry instead?
        m_event.StopOffer();
    }

    const owl::core::String m_instanceIdentifier;
    owl::kom::EventPublisher<Topic> m_event{m_serviceIdentifier, m_instanceIdentifier, "Event"};
    Topic initalFieldValue{4242};
    owl::kom::FieldPublisher<Topic> m_field{m_serviceIdentifier, m_instanceIdentifier, "Field", initalFieldValue};
    owl::kom::MethodServer computeSum{m_serviceIdentifier, m_instanceIdentifier, "Method"};
};