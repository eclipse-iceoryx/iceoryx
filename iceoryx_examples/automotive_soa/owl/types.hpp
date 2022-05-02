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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_TYPES_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_TYPES_HPP

#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/popo/sample.hpp"

#include "kom/sample_allocatee_ptr.hpp"

#include <future>
#include <memory>

namespace owl
{
class Runtime;
namespace core
{
template <typename T>
using Result = T;

using String = iox::cxx::string<100>;
} // namespace core

namespace kom
{
template <typename T>
using SamplePtr = iox::cxx::optional<iox::popo::Sample<const T>>;

using InstanceIdentifier = core::String;
using ServiceIdentifier = core::String;

struct FindServiceHandle
{
    // Only the Runtime shall be able to create handles, not the user
    friend class owl::Runtime;

  public:
    const ServiceIdentifier& GetServiceId() const noexcept
    {
        return m_serviceIdentifier;
    }
    const InstanceIdentifier& GetInstanceId() const noexcept
    {
        return m_instanceIdentifier;
    }

  private:
    FindServiceHandle(ServiceIdentifier serviceIdentifier, InstanceIdentifier instanceIdentifier) noexcept
        : m_serviceIdentifier(serviceIdentifier)
        , m_instanceIdentifier(instanceIdentifier)
    {
    }
    ServiceIdentifier m_serviceIdentifier;
    InstanceIdentifier m_instanceIdentifier;
};

struct ProxyHandleType
{
    // Only the Runtime shall be able to create handles, not the user
    friend class owl::Runtime;

  public:
    bool operator==(const ProxyHandleType& rhs) noexcept
    {
        return m_instanceIdentifier == rhs.m_instanceIdentifier;
    }
    const ServiceIdentifier& GetServiceId() const noexcept
    {
        return m_serviceIdentifier;
    }
    const InstanceIdentifier& GetInstanceId() const noexcept
    {
        return m_instanceIdentifier;
    }

  private:
    ProxyHandleType(ServiceIdentifier serviceIdentifier, InstanceIdentifier instanceIdentifier) noexcept
        : m_serviceIdentifier(serviceIdentifier)
        , m_instanceIdentifier(instanceIdentifier)
    {
    }
    ServiceIdentifier m_serviceIdentifier;
    InstanceIdentifier m_instanceIdentifier;
};

template <typename T>
using ServiceHandleContainer = iox::cxx::vector<T, 50U>;

template <typename T>
using FindServiceHandler = iox::cxx::function<void(ServiceHandleContainer<T>, FindServiceHandle)>;

using EventReceiveHandler = iox::cxx::function<void()>;

// We use the STL version as future is not available in hoofs
template <typename T>
using Future = std::future<T>;

// We use the STL version as promise is not available in hoofs
template <typename T>
using Promise = std::promise<T>;
} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_TYPES_HPP
