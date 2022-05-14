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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_MINIMAL_PROXY_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_MINIMAL_PROXY_HPP

#include "topic.hpp"

#include "owl/kom/event_subscriber.hpp"
#include "owl/kom/field_subscriber.hpp"
#include "owl/kom/method_client.hpp"
#include "owl/runtime.hpp"
#include "owl/types.hpp"

class MinimalProxy
{
  public:
    static constexpr char m_serviceIdentifier[] = "MinimalSkeleton";

    MinimalProxy(owl::kom::ProxyHandleType& handle) noexcept;
    ~MinimalProxy() noexcept = default;

    MinimalProxy(const MinimalProxy&) = delete;
    MinimalProxy(MinimalProxy&&) = delete;
    MinimalProxy& operator=(const MinimalProxy&) = delete;
    MinimalProxy& operator=(MinimalProxy&&) = delete;

    static owl::kom::FindServiceHandle StartFindService(owl::kom::FindServiceHandler<owl::kom::ProxyHandleType> handler,
                                                        owl::kom::InstanceIdentifier& instanceIdentifier) noexcept;

    static void StopFindService(owl::kom::FindServiceHandle handle) noexcept;

    static owl::kom::ServiceHandleContainer<owl::kom::ProxyHandleType>
    FindService(owl::kom::InstanceIdentifier& instanceIdentifier) noexcept;

    const owl::core::String m_instanceIdentifier;
    owl::kom::EventSubscriber<TimestampTopic1Byte> m_event{m_serviceIdentifier, m_instanceIdentifier, "Event"};
    owl::kom::FieldSubscriber<Topic> m_field{m_serviceIdentifier, m_instanceIdentifier, "Field"};
    owl::kom::MethodClient computeSum{m_serviceIdentifier, m_instanceIdentifier, "Method"};
};

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_MINIMAL_PROXY_HPP
