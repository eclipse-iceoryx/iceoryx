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

#include "ara/com/event_subscriber.hpp"
// #include "ara/com/field_subscriber.hpp"
// #include "ara/com/method_client.hpp"
#include "ara/types.hpp"

class MinimalProxy
{
  public:
    MinimalProxy() = default;
    MinimalProxy(const MinimalProxy&) = delete;
    MinimalProxy& operator=(const MinimalProxy&) = delete;

    static ara::com::FindServiceHandle StartFindService();
    static void StopFindService();
    static ara::com::ServiceHandleContainer<ara::com::FindServiceHandle>
    FindService(ara::core::String& InstanceIdentifier);

    ara::com::EventSubscriber<Topic> m_event{"MinimalSkeleton", "Instance", "Event"};
    // ara::com::FieldSubscriber<Topic> m_field{"MinimalSkeleton", "Instance", "Field"};
    // ara::com::MethodClient computeSum{"MinimalSkeleton", "Instance", "Method"};
};