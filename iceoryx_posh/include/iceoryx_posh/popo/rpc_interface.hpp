// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_RPC_INTERFACE_HPP
#define IOX_POSH_POPO_RPC_INTERFACE_HPP

#include "iceoryx_posh/popo/request.hpp"
#include "iceoryx_posh/popo/response.hpp"

namespace iox
{
namespace popo
{
///
/// @brief The RpcInterface class defines the interface used by the Request/Response class to make it generic.
/// This allows any client/server specialization to be stored as a reference by the Request/Response class.
/// It is also needed to avoid circular dependencies between Request/Response and Client/Server.
///
template <typename RpcType>
class RpcInterface
{
  public:
    virtual void send(RpcType&& sample) noexcept = 0;

    virtual ~RpcInterface(){};

  protected:
    RpcInterface() = default;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_RPC_INTERFACE_HPP
