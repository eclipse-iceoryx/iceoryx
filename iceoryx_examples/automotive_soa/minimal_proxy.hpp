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

#include "owl/kom/event_subscriber.hpp"
#include "owl/kom/field_subscriber.hpp"
#include "owl/kom/method_client.hpp"
#include "owl/runtime.hpp"
#include "owl/types.hpp"

class MinimalProxy
{
  public:
    /// @todo make c'tor of cxx::string constexpr'able
    static constexpr char m_serviceIdentifier[] = "MinimalSkeleton";
    /// @todo abstract type to be used in the runtime to remove the template
    struct HandleType // : public HandleBaseType
    {
        // Only the Runtime shall be able to create handles, not the user
        template <typename HandleType>
        friend class owl::Runtime;

      public:
        bool operator==(const HandleType& rhs) noexcept
        {
            return m_instanceIdentifier == rhs.m_instanceIdentifier;
        }
        const owl::kom::InstanceIdentifier& GetInstanceId() const noexcept
        {
            return m_instanceIdentifier;
        }

      private:
        HandleType(owl::kom::InstanceIdentifier instanceIdentifier) noexcept
            : m_instanceIdentifier(instanceIdentifier)
        {
        }
        owl::kom::InstanceIdentifier m_instanceIdentifier;
    };

    MinimalProxy(HandleType& handle)
        : m_instanceIdentifier(handle.GetInstanceId())
    {
        /// @todo sanity check on serviceIdentifier? not needed, ensured via type safety
    }
    MinimalProxy(const MinimalProxy&) = delete;
    MinimalProxy& operator=(const MinimalProxy&) = delete;

    static owl::kom::FindServiceHandle StartFindService(owl::kom::FindServiceHandler<HandleType> handler,
                                                        owl::kom::InstanceIdentifier& instanceIdentifier) noexcept
    {
        owl::kom::ServiceIdentifier serviceIdentifier{iox::cxx::TruncateToCapacity, m_serviceIdentifier};
        return owl::Runtime<HandleType>::GetInstance().StartFindService(handler, serviceIdentifier, instanceIdentifier);
    }

    static void StopFindService(owl::kom::FindServiceHandle handle) noexcept
    {
        owl::Runtime<HandleType>::GetInstance().StopFindService(handle);
    }

    static owl::kom::ServiceHandleContainer<HandleType> FindService(owl::core::String& instanceIdentifier) noexcept
    {
        owl::core::String serviceIdentifier{iox::cxx::TruncateToCapacity, m_serviceIdentifier};
        return owl::Runtime<HandleType>::GetInstance().FindService(serviceIdentifier, instanceIdentifier);
    }

    const owl::core::String m_instanceIdentifier;
    owl::kom::EventSubscriber<Topic> m_event{m_serviceIdentifier, m_instanceIdentifier, "Event"};
    owl::kom::FieldSubscriber<Topic> m_field{m_serviceIdentifier, m_instanceIdentifier, "Field"};
    owl::kom::MethodClient computeSum{m_serviceIdentifier, m_instanceIdentifier, "Method"};
};