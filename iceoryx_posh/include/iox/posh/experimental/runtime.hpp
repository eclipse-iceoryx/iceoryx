// Copyright (c) 2024 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#ifndef IOX_POSH_EXPERIMENTAL_RUNTIME_HPP
#define IOX_POSH_EXPERIMENTAL_RUNTIME_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/ipc_runtime_interface.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"
#include "iox/posh/experimental/publisher.hpp"
#include "iox/posh/experimental/subscriber.hpp"

namespace iox::posh::experimental
{

class Runtime;

class RuntimeBuilder
{
  public:
    explicit RuntimeBuilder(const RuntimeName_t& name) noexcept;

    enum class Error
    {
        RUNTIME_STILL_ACTIVE,
        IPC_CHANNEL_CREATION_FAILED,
        TIMEOUT,
        REGISTRATION_FAILED,
    };

    IOX_BUILDER_PARAMETER(uint16_t, roudi_id, roudi::DEFAULT_UNIQUE_ROUDI_ID)

    IOX_BUILDER_PARAMETER(units::Duration, roudi_registration_timeout, units::Duration::zero())

    IOX_BUILDER_PARAMETER(bool, shares_process_with_roudi, false)

  public:
    expected<void, Error> create(optional<Runtime>& runtime_container) noexcept;

  private:
    RuntimeName_t m_name;
};

class Runtime
{
  public:
    PublisherBuilder publisher(const capro::ServiceDescription& service_description) noexcept;

    SubscriberBuilder subscriber(const capro::ServiceDescription& service_description) noexcept;

  private:
    friend class RuntimeBuilder;
    friend class optional<Runtime>;
    Runtime(const RuntimeName_t& name,
            runtime::RuntimeLocation location,
            runtime::IpcRuntimeInterface&& runtime_interface) noexcept;

  private:
    runtime::PoshRuntimeImpl m_runtime;
};

} // namespace iox::posh::experimental

namespace iox
{
template <>
constexpr posh::experimental::RuntimeBuilder::Error
from<runtime::IpcRuntimeInterface::Error, posh::experimental::RuntimeBuilder::Error>(
    runtime::IpcRuntimeInterface::Error e) noexcept
{
    using namespace posh::experimental;
    using namespace runtime;
    switch (e)
    {
    case IpcRuntimeInterface::Error::CANNOT_CREATE_APPLICATION_CHANNEL:
        return RuntimeBuilder::Error::IPC_CHANNEL_CREATION_FAILED;
    case IpcRuntimeInterface::Error::TIMEOUT_WAITING_FOR_ROUDI:
        return RuntimeBuilder::Error::TIMEOUT;
    case IpcRuntimeInterface::Error::SENDING_REQUEST_TO_ROUDI_FAILED:
        [[fallthrough]];
    case IpcRuntimeInterface::Error::NO_RESPONSE_FROM_ROUDI:
        return RuntimeBuilder::Error::REGISTRATION_FAILED;
    }
}
} // namespace iox

#endif // IOX_HOOFS_REPORTING_LOGGING_HPP
