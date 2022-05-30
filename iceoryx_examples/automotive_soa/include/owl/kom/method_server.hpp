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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_SERVER_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_SERVER_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/server.hpp"

#include "owl/types.hpp"
#include "topic.hpp"

namespace owl
{
namespace kom
{
class MethodServer
{
  public:
    MethodServer(const ServiceIdentifier& service,
                 const InstanceIdentifier& instance,
                 const MethodIdentifier& method) noexcept;

    ~MethodServer() noexcept;

    MethodServer(const MethodServer&) = delete;
    MethodServer(MethodServer&&) = delete;
    MethodServer& operator=(const MethodServer&) = delete;
    MethodServer& operator=(MethodServer&&) = delete;

    Future<AddResponse> computeSum(uint64_t addend1, uint64_t addend2);

  private:
    static void onRequestReceived(iox::popo::Server<AddRequest, AddResponse>* server, MethodServer* self) noexcept;

    uint64_t computeSumInternal(uint64_t addend1, uint64_t addend2) noexcept;

    //! [MethodServer members]
    iox::popo::Server<AddRequest, AddResponse> m_server;
    iox::popo::Listener m_listener;
    //! [MethodServer members]
};
} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_SERVER_HPP
