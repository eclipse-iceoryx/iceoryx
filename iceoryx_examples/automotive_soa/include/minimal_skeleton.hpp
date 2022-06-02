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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_MINIMAL_SKELETON_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_MINIMAL_SKELETON_HPP

#include "topic.hpp"

#include "owl/kom/event_publisher.hpp"
#include "owl/kom/field_publisher.hpp"
#include "owl/kom/method_server.hpp"
#include "owl/types.hpp"

/// @note Skeleton classes are typically generated
class MinimalSkeleton
{
  public:
    static constexpr char SERVICE_IDENTIFIER[] = "ExampleSkeleton";

    MinimalSkeleton(const owl::kom::InstanceIdentifier& instanceIdentifier) noexcept;
    ~MinimalSkeleton() noexcept;

    MinimalSkeleton(const MinimalSkeleton&) = delete;
    MinimalSkeleton(MinimalSkeleton&&) = delete;
    MinimalSkeleton& operator=(const MinimalSkeleton&) = delete;
    MinimalSkeleton& operator=(MinimalSkeleton&&) = delete;

    void Offer() noexcept;
    void StopOffer() noexcept;

    const owl::kom::InstanceIdentifier m_instanceIdentifier;
    owl::kom::EventPublisher<TimestampTopic1Byte> m_event{SERVICE_IDENTIFIER, m_instanceIdentifier, "Event"};
    static constexpr Topic initalFieldValue{4242};
    owl::kom::FieldPublisher<Topic> m_field{SERVICE_IDENTIFIER, m_instanceIdentifier, "Field", initalFieldValue};
    owl::kom::MethodServer computeSum{SERVICE_IDENTIFIER, m_instanceIdentifier, "Method"};
};

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_MINIMAL_SKELETON_HPP
