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

#include "owl/kom/method_client.hpp"

namespace owl
{
namespace kom
{
MethodClient::MethodClient(const ServiceIdentifier& service,
                           const InstanceIdentifier& instance,
                           const MethodIdentifier& method) noexcept
    : m_client({service, instance, method})
{
    m_waitset.attachState(m_client, iox::popo::ClientState::HAS_RESPONSE).expect("Failed to attach client!");
}

MethodClient::~MethodClient() noexcept
{
    m_waitset.detachState(m_client, iox::popo::ClientState::HAS_RESPONSE);

    // Wait here if a thread or callback is still running
    while (m_threadsRunning > 0)
    {
    }
    std::lock_guard<iox::posix::mutex> guard(m_mutex);
}

// If we call the operator() twice shortly after each other, once the response of the first request has not yet
// arrived, we have a problem
//! [MethodClient send request]
Future<AddResponse> MethodClient::operator()(uint64_t addend1, uint64_t addend2)
{
    bool requestSuccessfullySent{false};
    m_client.loan()
        .and_then([&](auto& request) {
            request.getRequestHeader().setSequenceId(m_sequenceId);
            request->addend1 = addend1;
            request->addend2 = addend2;
            request.send().and_then([&]() { requestSuccessfullySent = true; }).or_else([](auto& error) {
                std::cerr << "Could not send request! Error: " << error << std::endl;
            });
        })
        .or_else([&](auto& error) {
            std::cerr << "Could not allocate request! Error: " << error << std::endl;
            requestSuccessfullySent = false;
        });

    if (!requestSuccessfullySent)
    {
        return Future<AddResponse>();
    }
    //! [MethodClient send request]

    Promise<AddResponse> promise;
    auto future = promise.get_future();

    m_threadsRunning++;
    // Typically you would e.g. use a worker pool here, for simplicity we use a plain thread
    std::thread(
        [&](Promise<AddResponse>&& promise) {
            // Avoid race if MethodClient d'tor is called while this thread is still running
            std::lock_guard<iox::posix::mutex> guard(m_mutex);

            auto notificationVector = m_waitset.timedWait(iox::units::Duration::fromSeconds(5));

            if (notificationVector.empty())
            {
                std::cerr << "WaitSet ran into timeout when trying to receive response!" << std::endl;
            }

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
            m_threadsRunning--;
        },
        std::move(promise))
        .detach();
    return future;
}

} // namespace kom
} // namespace owl
