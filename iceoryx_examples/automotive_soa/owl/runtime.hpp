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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP

#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"

#include "types.hpp"

#include <string>

namespace owl
{
class Runtime
{
  public:
    static Runtime& GetInstance(const std::string& name) noexcept
    {
        iox::runtime::PoshRuntime::initRuntime(iox::RuntimeName_t(iox::cxx::TruncateToCapacity, name));
        static Runtime runtime;
        return runtime;
    }

    static Runtime& GetInstance() noexcept
    {
        static Runtime runtime;
        return runtime;
    }

    owl::kom::ServiceHandleContainer<owl::kom::FindServiceHandle>
    FindService(owl::core::String& InstanceIdentifier) noexcept
    {
        owl::kom::ServiceHandleContainer<owl::kom::FindServiceHandle> serviceContainer;
        m_discovery.findService(
            iox::cxx::nullopt,
            InstanceIdentifier,
            iox::cxx::nullopt,
            [&](auto& service) { serviceContainer.push_back(service); },
            iox::popo::MessagingPattern::PUB_SUB);

        return serviceContainer;
    }

  private:
    explicit Runtime() noexcept = default;

    iox::runtime::ServiceDiscovery m_discovery;
    iox::popo::Listener m_listener;
};
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_RUNTIME_HPP
