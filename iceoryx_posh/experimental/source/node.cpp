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

expected<Node, NodeBuilderError> NodeBuilder::create() noexcept
{
    auto location = m_shares_address_space_with_roudi ? runtime::RuntimeLocation::SAME_PROCESS_LIKE_ROUDI
                                                      : runtime::RuntimeLocation::SEPARATE_PROCESS_FROM_ROUDI;
    auto ipcRuntimeIterface = runtime::IpcRuntimeInterface::create(m_name, m_roudi_registration_timeout);
    if (ipcRuntimeIterface.has_error())
    {
        return err(into<NodeBuilderError>(ipcRuntimeIterface.error()));
    }
    return ok(Node{m_name, location, std::move(ipcRuntimeIterface.value())});
}

Node::Node(const NodeName_t& name,
           runtime::RuntimeLocation location,
           runtime::IpcRuntimeInterface&& runtime_interface) noexcept
    : m_runtime(unique_ptr<runtime::PoshRuntime>{
        new runtime::PoshRuntimeImpl{make_optional<const NodeName_t*>(&name), location, std::move(runtime_interface)},
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
