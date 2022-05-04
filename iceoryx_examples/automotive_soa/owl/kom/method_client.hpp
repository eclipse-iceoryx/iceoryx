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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_CLIENT_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_CLIENT_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/client.hpp"

#include "owl/types.hpp"

#include <memory>
#include <thread>
#include <utility>

namespace owl
{
namespace kom
{
class MethodClient
{
  public:
    MethodClient(const core::String& service, const core::String& instance, const core::String& event)
        : m_client({service, instance, event})
    {
        m_waitset.attachState(m_client, iox::popo::ClientState::HAS_RESPONSE).or_else([](auto) {
            std::cerr << "failed to attach client" << std::endl;
            std::exit(EXIT_FAILURE);
        });
    }

    ~MethodClient()
    {
        m_waitset.detachState(m_client, iox::popo::ClientState::HAS_RESPONSE);
        /// @todo #1332 wait for mutex
    }

    MethodClient(const MethodClient&) = delete;
    MethodClient(MethodClient&&) = delete;
    MethodClient& operator=(const MethodClient&) = delete;
    MethodClient& operator=(MethodClient&&) = delete;

    owl::kom::Future<AddResponse> operator()(uint64_t addend1, uint64_t addend2)
    {
        // If we call the operator() twice shortly after each other, once the response of the first request has not yet
        // arrived, we have a problem
        bool requestSuccessfullySend{false};
        m_client.loan()
            .and_then([&](auto& request) {
                request.getRequestHeader().setSequenceId(m_sequenceId);
                request->addend1 = addend1;
                request->addend2 = addend2;
                request.send().and_then([&]() { requestSuccessfullySend = true; }).or_else([](auto& error) {
                    std::cerr << "Could not send Request! Error: " << error << std::endl;
                });
            })
            .or_else([&](auto& error) {
                std::cerr << "Could not allocate Request! Error: " << error << std::endl;
                requestSuccessfullySend = false;
            });

        if (!requestSuccessfullySend)
        {
            return owl::kom::Future<AddResponse>();
        }

        Promise<AddResponse> promise;
        auto future = promise.get_future();
        // Typically you would e.g. use a worker pool here, for simplicity we use a plain thread
        std::thread(
            [&](Promise<AddResponse>&& promise) {
                /// @todo #1332 we are working on a proxy object that can be destroyed during runtime
                /// How to ensure safe operation?

                /// @todo #1332 add and lock mutex to wait in d'tor

                auto notificationVector = m_waitset.timedWait(iox::units::Duration::fromSeconds(5));

                for (auto& notification : notificationVector)
                {
                    if (notification->doesOriginateFrom(&m_client))
                    {
                        while (m_client.take().and_then([&](const auto& response) {
                            auto receivedSequenceId = response.getResponseHeader().getSequenceId();
                            if (receivedSequenceId == m_sequenceId)
                            {
                                AddResponse result{response->sum};
                                m_sequenceId++;
                                promise.set_value_at_thread_exit(result);
                            }
                            else
                            {
                                std::cerr << "Got Response with outdated sequence ID! Expected = " << m_sequenceId
                                          << "; Actual = " << receivedSequenceId << "!" << std::endl;
                                std::terminate();
                            }
                        }))
                        {
                        }
                    }
                }
            },
            std::move(promise))
            .detach();
        return future;
    }

  private:
    iox::popo::Client<AddRequest, AddResponse> m_client;
    std::atomic<int64_t> m_sequenceId{0};
    iox::popo::WaitSet<> m_waitset;
};
} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_METHOD_CLIENT_HPP
