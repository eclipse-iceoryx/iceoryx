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

#ifndef IOX_POSH_RPC_INTERFACE_HPP
#define IOX_POSH_RPC_INTERFACE_HPP

#include "iox/expected.hpp"

namespace iox
{
namespace popo
{
/// @brief The RpcInterface class defines the request/response interface used by the Request/Response
/// classes to make them generic. This allows any client or server specialization to be stored as a reference
/// by the Request/Response class. It is also needed to avoid circular dependencies between Request/Response
/// and Client/Sever.
/// @tparam RpcType is either Request<T> for the client or Response<T> for the server
template <typename RpcType, typename SendErrorEnum>
class RpcInterface
{
  public:
    /// @brief Sends the given Request<T> or Response<T> via the type which implements this interface
    /// @param[in] rpcData is the actual Request<T> or Response<T> instance
    /// @return Error if sending was not successful
    virtual expected<void, SendErrorEnum> send(RpcType&& rpcData) noexcept = 0;

  protected:
    RpcInterface() = default;
};
} // namespace popo
} // namespace iox

#endif // IOX_POSH_RPC_INTERFACE_HPP
