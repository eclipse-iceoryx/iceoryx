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

#include "iox/posh/experimental/node.hpp"
#include "iceoryx_posh/internal/runtime/posh_runtime_impl.hpp"

namespace iox::posh::experimental
{
NodeBuilder::NodeBuilder(const NodeName_t& name) noexcept
    : m_name(name)
{
}

NodeBuilder&& NodeBuilder::domain_id(const DomainId domainId) && noexcept
{
    m_domain_id.emplace(domainId);
    return std::move(*this);
}

NodeBuilder&& NodeBuilder::domain_id_from_env() && noexcept
{
    m_domain_id.reset();
    // JUSTIFICATION getenv is required for the functionality of this function; see also declaration in header
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    if (const auto* domain_id_string = std::getenv("IOX_DOMAIN_ID"))
    {
        iox::convert::from_string<uint16_t>(domain_id_string)
            .and_then([this](const auto& env_domain_id) { m_domain_id.emplace(env_domain_id); })
            .or_else([&domain_id_string]() {
                IOX_LOG(INFO, "Invalid value for 'IOX_DOMAIN_ID' environment variable!'");
                IOX_LOG(INFO, "Found: '" << domain_id_string << "'! Allowed are integer from '0' to '65535'!");
            });
    }
    return std::move(*this);
}

NodeBuilder&& NodeBuilder::domain_id_from_env_or(const DomainId domainId) && noexcept
{
    std::move(*this).domain_id_from_env();
    if (!m_domain_id.has_value())
    {
        IOX_LOG(INFO,
                "Could not get domain ID from 'IOX_DOMAIN_ID' and using '"
                    << static_cast<DomainId::value_type>(domainId) << "' as fallback!");
        m_domain_id.emplace(domainId);
    }
    return std::move(*this);
}

NodeBuilder&& NodeBuilder::domain_id_from_env_or_default() && noexcept
{
    return std::move(*this).domain_id_from_env_or(DEFAULT_DOMAIN_ID);
}

expected<Node, NodeBuilderError> NodeBuilder::create() noexcept
{
    if (!m_domain_id.has_value())
    {
        return err(NodeBuilderError::INVALID_OR_NO_DOMAIN_ID);
    }
    auto domain_id = m_domain_id.value();

    auto ipcRuntimeInterfaceResult =
        runtime::IpcRuntimeInterface::create(m_name, domain_id, m_roudi_registration_timeout);
    if (ipcRuntimeInterfaceResult.has_error())
    {
        return err(into<NodeBuilderError>(ipcRuntimeInterfaceResult.error()));
    }

    auto& ipcRuntimeInterface = ipcRuntimeInterfaceResult.value();

    optional<runtime::SharedMemoryUser> shmInterface;

    // in case the runtime is located in the same process as RouDi the shm segments are already opened;
    // also in case of the RouDiEnv this would close the shm on destruction of the runtime which is also
    // not desired; therefore open the shm segments only when the runtime lives in a different process from RouDi
    if (!m_shares_address_space_with_roudi)
    {
        auto shmInterfaceResult =
            runtime::SharedMemoryUser::create(domain_id,
                                              ipcRuntimeInterface.getSegmentId(),
                                              ipcRuntimeInterface.getShmTopicSize(),
                                              ipcRuntimeInterface.getSegmentManagerAddressOffset())
                .and_then([&shmInterface](auto& value) { shmInterface.emplace(std::move(value)); });
        if (shmInterfaceResult.has_error())
        {
            return err(into<NodeBuilderError>(shmInterfaceResult.error()));
        }
    }

    return ok(Node{m_name, std::move(ipcRuntimeInterface), std::move(shmInterface)});
}

Node::Node(const NodeName_t& name,
           runtime::IpcRuntimeInterface&& runtime_interface,
           optional<runtime::SharedMemoryUser>&& ipc_interface) noexcept
    : m_runtime(unique_ptr<runtime::PoshRuntime>{
        new runtime::PoshRuntimeImpl{make_optional<const NodeName_t*>(&name),
                                     {std::move(runtime_interface), std::move(ipc_interface)}},
        [&](auto* const rt) { delete rt; }})
{
}

PublisherBuilder Node::publisher(const ServiceDescription& service_description) noexcept
{
    return PublisherBuilder{*m_runtime.get(), service_description};
}

SubscriberBuilder Node::subscriber(const ServiceDescription& service_description) noexcept
{
    return SubscriberBuilder{*m_runtime.get(), service_description};
}

WaitSetBuilder Node::wait_set() noexcept
{
    return WaitSetBuilder{*m_runtime.get()};
}

} // namespace iox::posh::experimental
