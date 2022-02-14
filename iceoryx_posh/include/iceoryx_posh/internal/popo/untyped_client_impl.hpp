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

#ifndef IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_HPP
#define IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/base_client.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
template <typename BaseClientT = BaseClient<>>
class UntypedClientImpl : public BaseClientT
{
  public:
    explicit UntypedClientImpl(const capro::ServiceDescription& service,
                               const ClientOptions& clientOptions = {}) noexcept;

    cxx::expected<void*, AllocationError> loan(const uint32_t payloadSize, const uint32_t payloadAlignment) noexcept;
    void releaseRequest(void* const requestPayload) noexcept;
    void send(void* const requestPayload) noexcept;

    cxx::expected<const void*, ChunkReceiveResult> take() noexcept;
    void releaseResponse(const void* const responsePayload) noexcept;

  protected:
    using BaseClientT::port;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/untyped_client_impl.inl"

#endif // IOX_POSH_POPO_UNTYPED_CLIENT_IMPL_HPP
