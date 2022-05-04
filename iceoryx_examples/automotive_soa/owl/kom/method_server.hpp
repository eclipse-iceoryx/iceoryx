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

#include <memory>
#include <utility>

namespace owl
{
namespace kom
{
class MethodServer
{
  public:
    MethodServer(const core::String& service, const core::String& instance, const core::String& event) noexcept
        : m_server({service, instance, event})
    {
        m_listener
            .attachEvent(m_server,
                         iox::popo::ServerEvent::REQUEST_RECEIVED,
                         iox::popo::createNotificationCallback(onRequestReceived, *this))
            .or_else([](auto) {
                std::cerr << "unable to attach server" << std::endl;
                std::exit(EXIT_FAILURE);
            });
    }

    ~MethodServer() noexcept
    {
        m_listener.detachEvent(m_server, iox::popo::ServerEvent::REQUEST_RECEIVED);
    }

    MethodServer(const MethodServer&) = delete;
    MethodServer(MethodServer&&) = delete;
    MethodServer& operator=(const MethodServer&) = delete;
    MethodServer& operator=(MethodServer&&) = delete;

    owl::kom::Future<AddResponse> computeSum(uint64_t addend1, uint64_t addend2)
    {
        // for simplicity we don't create thread here and make the call sychronous
        Promise<AddResponse> promise;
        promise.set_value({computeSumInternal(addend1, addend2)});
        return promise.get_future();
    }

  private:
    static void onRequestReceived(iox::popo::Server<AddRequest, AddResponse>* server, MethodServer* self) noexcept
    {
        while (server->take().and_then([&](const auto& request) {
            server->loan(request)
                .and_then([&](auto& response) {
                    response->sum = self->computeSumInternal(request->addend1, request->addend2);
                    response.send().or_else(
                        [&](auto& error) { std::cout << "Could not send Response! Error: " << error << std::endl; });
                })
                .or_else(
                    [](auto& error) { std::cout << "Could not allocate Response! Error: " << error << std::endl; });
        }))
        {
        }
    }

    uint64_t computeSumInternal(uint64_t addend1, uint64_t addend2) noexcept
    {
        return addend1 + addend2;
    }

    iox::popo::Server<AddRequest, AddResponse> m_server;
    iox::popo::Listener m_listener;
};
} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_SERVER_HPP
