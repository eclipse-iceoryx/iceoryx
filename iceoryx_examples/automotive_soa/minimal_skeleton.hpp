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
    MinimalSkeleton() = default;
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

    owl::kom::EventPublisher<Topic> m_event{"MinimalSkeleton", "Instance", "Event"};
    Topic initalFieldValue{4242};
    owl::kom::FieldPublisher<Topic> m_field{"MinimalSkeleton", "Instance", "Field", initalFieldValue};
    owl::kom::MethodServer computeSum{"MinimalSkeleton", "Instance", "Method"};
};