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

#ifndef IOX_POSH_POPO_UNTYPED_SERVER_IMPL_HPP
#define IOX_POSH_POPO_UNTYPED_SERVER_IMPL_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/base_server.hpp"
#include "iceoryx_posh/popo/server_options.hpp"

namespace iox
{
namespace popo
{
template <typename BaseServerT = BaseServer<>>
class UntypedServerImpl : public BaseServerT
{
  public:
    explicit UntypedServerImpl(const capro::ServiceDescription& service,
                               const ServerOptions& serverOptions = {}) noexcept;

    cxx::expected<const void*, ServerRequestResult> take() noexcept;
    void releaseRequest(const void* const requestPayload) noexcept;

    cxx::expected<void*, AllocationError>
    loan(const RequestHeader* requestHeader, const uint32_t payloadSize, const uint32_t payloadAlignment) noexcept;
    void send(void* const responsePayload) noexcept;
    void freeResponse(void* const responsePayload) noexcept;

  protected:
    using BaseServerT::port;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_server_impl.inl"

#endif // IOX_POSH_POPO_UNTYPED_SERVER_IMPL_HPP
