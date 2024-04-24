// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_POSH_EXPERIMENTAL_NODE_HPP
#define IOX_POSH_EXPERIMENTAL_NODE_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/runtime/ipc_runtime_interface.hpp"
#include "iceoryx_posh/internal/runtime/shared_memory_user.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/assertions.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"
#include "iox/posh/experimental/publisher.hpp"
#include "iox/posh/experimental/subscriber.hpp"
#include "iox/posh/experimental/wait_set.hpp"
#include "iox/unique_ptr.hpp"

namespace iox::posh::experimental
{
using capro::ServiceDescription;

class Node;

enum class NodeBuilderError : uint8_t
{
    INVALID_OR_NO_DOMAIN_ID,
    IPC_CHANNEL_CREATION_FAILED,
    TIMEOUT,
    REGISTRATION_FAILED,
    SHM_MAPPING_ERROR,
    RELATIVE_POINTER_MAPPING_ERROR,
    TOO_MANY_SHM_SEGMENTS,
};

/// @brief A builder for a 'Node' which is th entry point to create publisher, subscriber, wait sets, etc.
/// @note For testing purposes there is also the 'RouDiEnvNodeBuilder'
class NodeBuilder
{
  public:
    /// @brief Initiates the node builder
    /// @param[in] name is the name the node is identified with; The name must be unique across processes
    explicit NodeBuilder(const NodeName_t& name) noexcept;

    /// @brief Determines the time to wait for registration at RouDi
    IOX_BUILDER_PARAMETER(units::Duration, roudi_registration_timeout, units::Duration::zero())

    /// @brief Indicates whether the node shares the address space with 'RouDi', e.g. in single process applications or
    /// tests
    IOX_BUILDER_PARAMETER(bool, shares_address_space_with_roudi, false)

  public:
    /// @brief Determines which domain to use to register to a RouDi instance
    /// @param[in] domain_id to be used as domain ID
    NodeBuilder&& domain_id(const DomainId domainId) && noexcept;

    /// @brief Determines which domain to use to register to a RouDi instance by using the one specified by
    /// 'IOX_DOMAIN_ID'. If the environment variable is not set or invalid, the creation of the 'Node' will fail.
    /// @note The function uses 'iox_getenv_s' which is only conditionally thread safe as long as only 'iox_getenv',
    /// 'iox_setenv' and 'iox_unsetenv' are used and none of 'getenv', 'setenv', 'unsetenv' and 'putenv' directly.
    /// Using the POSIX functions directly can result in undefined behavior when it is called from multiple threads
    /// or the env variable is changed while the function holds a pointer to the data. For this reason only the 'iox_*'
    /// function should be used or if one does not have full control of the environment, these functions should only
    /// be used in the startup phase of the application and only in the main thread.
    NodeBuilder&& domain_id_from_env() && noexcept;

    /// @brief Determines which domain to use to register to a RouDi instance by using the one specified by
    /// 'IOX_DOMAIN_ID' or the one by 'value' if the environment variable is not set or invalid.
    /// @param[in] domain_id to be used as domain ID if the 'IOX_DOMAIN_ID' environment variable is not set or invalid
    /// @note The function uses 'iox_getenv_s' which is only conditionally thread safe as long as only 'iox_getenv',
    /// 'iox_setenv' and 'iox_unsetenv' are used and none of 'getenv', 'setenv', 'unsetenv' and 'putenv' directly.
    /// Using the POSIX functions directly can result in undefined behavior when it is called from multiple threads
    /// or the env variable is changed while the function holds a pointer to the data. For this reason only the 'iox_*'
    /// function should be used or if one does not have full control of the environment, these functions should only
    /// be used in the startup phase of the application and only in the main thread.
    NodeBuilder&& domain_id_from_env_or(const DomainId domainId) && noexcept;

    /// @brief Determines which domain to use to register to a RouDi instance using the one specified by
    /// 'IOX_DOMAIN_ID' or the the default Domain ID if the environment variable is not set
    /// @note The function uses 'iox_getenv_s' which is only conditionally thread safe as long as only 'iox_getenv',
    /// 'iox_setenv' and 'iox_unsetenv' are used and none of 'getenv', 'setenv', 'unsetenv' and 'putenv' directly.
    /// Using the POSIX functions directly can result in undefined behavior when it is called from multiple threads
    /// or the env variable is changed while the function holds a pointer to the data. For this reason only the 'iox_*'
    /// function should be used or if one does not have full control of the environment, these functions should only
    /// be used in the startup phase of the application and only in the main thread.
    NodeBuilder&& domain_id_from_env_or_default() && noexcept;

    expected<Node, NodeBuilderError> create() noexcept;

  private:
    NodeName_t m_name;
    optional<DomainId> m_domain_id{DEFAULT_DOMAIN_ID};
};

/// @brief Entry point to create publisher, subscriber, wait sets, etc.
class Node
{
  public:
    /// @brief Initiates a 'PublisherBuilder'
    /// @param[in] service_description for the publisher
    PublisherBuilder publisher(const ServiceDescription& service_description) noexcept;

    /// @brief Initiates a 'SubscriberBuilder'
    /// @param[in] service_description for the subscriber
    SubscriberBuilder subscriber(const ServiceDescription& service_description) noexcept;

    /// @brief Initiates a 'WaitSetBuilder'
    WaitSetBuilder wait_set() noexcept;

  private:
    friend class NodeBuilder;
    Node(const NodeName_t& name,
         runtime::IpcRuntimeInterface&& runtime_interface,
         optional<runtime::SharedMemoryUser>&&) noexcept;

  private:
    unique_ptr<runtime::PoshRuntime> m_runtime;
};

} // namespace iox::posh::experimental

namespace iox
{
template <>
inline constexpr posh::experimental::NodeBuilderError
from<runtime::IpcRuntimeInterfaceError, posh::experimental::NodeBuilderError>(
    runtime::IpcRuntimeInterfaceError e) noexcept
{
    using namespace posh::experimental;
    using namespace runtime;
    switch (e)
    {
    case IpcRuntimeInterfaceError::CANNOT_CREATE_APPLICATION_CHANNEL:
        return NodeBuilderError::IPC_CHANNEL_CREATION_FAILED;
    case IpcRuntimeInterfaceError::TIMEOUT_WAITING_FOR_ROUDI:
        return NodeBuilderError::TIMEOUT;
    case IpcRuntimeInterfaceError::SENDING_REQUEST_TO_ROUDI_FAILED:
        [[fallthrough]];
    case IpcRuntimeInterfaceError::NO_RESPONSE_FROM_ROUDI:
        return NodeBuilderError::REGISTRATION_FAILED;
    }

    // just to prevent a warning regarding not returning from a non-void function; IOX_UNREACHABLE does not work since
    // this is a constexpr
    return NodeBuilderError::REGISTRATION_FAILED;
}

template <>
inline constexpr posh::experimental::NodeBuilderError
from<runtime::SharedMemoryUserError, posh::experimental::NodeBuilderError>(runtime::SharedMemoryUserError e) noexcept
{
    using namespace posh::experimental;
    using namespace runtime;
    switch (e)
    {
    case SharedMemoryUserError::SHM_MAPPING_ERROR:
        return NodeBuilderError::SHM_MAPPING_ERROR;
    case SharedMemoryUserError::RELATIVE_POINTER_MAPPING_ERROR:
        return NodeBuilderError::RELATIVE_POINTER_MAPPING_ERROR;
    case SharedMemoryUserError::TOO_MANY_SHM_SEGMENTS:
        return NodeBuilderError::TOO_MANY_SHM_SEGMENTS;
    }

    // just to prevent a warning regarding not returning from a non-void function; IOX_UNREACHABLE does not work since
    // this is a constexpr
    return NodeBuilderError::REGISTRATION_FAILED;
}
} // namespace iox

#endif // IOX_POSH_EXPERIMENTAL_NODE_HPP
