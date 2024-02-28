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

NodeBuilder&& NodeBuilder::roudi_id(const uint16_t value) && noexcept
{
    m_roudi_id.emplace(value);
    return std::move(*this);
}

NodeBuilder&& NodeBuilder::roudi_id_from_env() && noexcept
{
    m_roudi_id.reset();
    // JUSTIFICATION getenv is required for the functionality of this function; see also declaration in header
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    if (const auto* roudi_id_string = std::getenv("IOX_ROUDI_ID"))
    {
        iox::convert::from_string<uint16_t>(roudi_id_string)
            .and_then([this](const auto& env_roudi_id) { m_roudi_id.emplace(env_roudi_id); })
            .or_else([&roudi_id_string]() {
                IOX_LOG(INFO, "Invalid value for 'IOX_ROUDI_ID' environment variable!'");
                IOX_LOG(INFO, "Found: '" << roudi_id_string << "'! Allowed are integer from '0' to '65535'!");
            });
    }
    return std::move(*this);
}

NodeBuilder&& NodeBuilder::roudi_id_from_env_or(const uint16_t value) && noexcept
{
    std::move(*this).roudi_id_from_env();
    if (!m_roudi_id.has_value())
    {
        IOX_LOG(INFO, "Could not get RouDi ID from 'IOX_ROUDI_ID' and using '" << value << "' as fallback!");
        m_roudi_id.emplace(value);
    }
    return std::move(*this);
}

NodeBuilder&& NodeBuilder::roudi_id_from_env_or_default() && noexcept
{
    return std::move(*this).roudi_id_from_env_or(roudi::DEFAULT_UNIQUE_ROUDI_ID);
}

expected<Node, NodeBuilderError> NodeBuilder::create() noexcept
{
    if (!m_roudi_id.has_value())
    {
        return err(NodeBuilderError::INVALID_OR_NO_ROUDI_ID);
    }
    auto roudi_id = m_roudi_id.value();

    auto location = m_shares_address_space_with_roudi ? runtime::RuntimeLocation::SAME_PROCESS_LIKE_ROUDI
                                                      : runtime::RuntimeLocation::SEPARATE_PROCESS_FROM_ROUDI;
    auto ipcRuntimeIterface = runtime::IpcRuntimeInterface::create(m_name, roudi_id, m_roudi_registration_timeout);
    if (ipcRuntimeIterface.has_error())
    {
        return err(into<NodeBuilderError>(ipcRuntimeIterface.error()));
    }
    return ok(Node{m_name, roudi_id, location, std::move(ipcRuntimeIterface.value())});
}

Node::Node(const NodeName_t& name,
           const uint16_t uniqueRouDiId,
           const runtime::RuntimeLocation location,
           runtime::IpcRuntimeInterface&& runtime_interface) noexcept
    : m_runtime(unique_ptr<runtime::PoshRuntime>{
        new runtime::PoshRuntimeImpl{
            make_optional<const NodeName_t*>(&name), uniqueRouDiId, location, std::move(runtime_interface)},
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
