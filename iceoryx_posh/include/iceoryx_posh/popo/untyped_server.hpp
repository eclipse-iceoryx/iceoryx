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

#ifndef IOX_POSH_POPO_UNTYPED_SERVER_HPP
#define IOX_POSH_POPO_UNTYPED_SERVER_HPP

#include "iceoryx_posh/internal/popo/untyped_server_impl.hpp"

namespace iox::posh::experimental
{
class ServerBuilder;
}

namespace iox
{
namespace popo
{
class UntypedServer : public UntypedServerImpl<>
{
    using Impl = UntypedServerImpl<>;

  public:
    using UntypedServerImpl<>::UntypedServerImpl;

    virtual ~UntypedServer() noexcept
    {
        Impl::m_trigger.reset();
    }

  private:
    friend class iox::posh::experimental::ServerBuilder;

    explicit UntypedServer(typename UntypedServerImpl<>::PortType&& port) noexcept
        : UntypedServerImpl<>(std::move(port))
    {
    }
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_UNTYPED_SERVER_HPP
