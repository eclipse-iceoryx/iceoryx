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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_SUBSCRIBER_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_SUBSCRIBER_HPP

#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"

#include "owl/types.hpp"

#include <limits>
#include <memory>

namespace owl
{
namespace kom
{
template <typename T>
class FieldSubscriber
{
  public:
    using FieldType = T;
    static constexpr uint64_t QUEUE_CAPACITY{1U};
    static constexpr uint64_t HISTORY_REQUEST{1U};

    FieldSubscriber(const core::String& service, const core::String& instance, const core::String& event) noexcept
        : m_subscriber({service, instance, event}, {QUEUE_CAPACITY, HISTORY_REQUEST})
        , m_client({service, instance, event})
    {
        m_waitset.attachState(m_client, iox::popo::ClientState::HAS_RESPONSE).or_else([](auto) {
            std::cerr << "failed to attach client" << std::endl;
            std::exit(EXIT_FAILURE);
        });
    }

    ~FieldSubscriber()
    {
        m_waitset.detachState(m_client, iox::popo::ClientState::HAS_RESPONSE);
        // Wait here if a callback is still running
        std::lock_guard<iox::posix::mutex> guard(m_mutex);
    }

    FieldSubscriber(const FieldSubscriber&) = delete;
    FieldSubscriber(FieldSubscriber&&) = delete;
    FieldSubscriber& operator=(const FieldSubscriber&) = delete;
    FieldSubscriber& operator=(FieldSubscriber&&) = delete;

    template <typename Callable>
    owl::core::Result<size_t> GetNewSamples(Callable&& callable,
                                            size_t maxNumberOfSamples = std::numeric_limits<size_t>::max()) noexcept
    {
        IOX_DISCARD_RESULT(maxNumberOfSamples);

        owl::core::Result<size_t> numberOfSamples{0};

        while (m_subscriber.take()
                   .and_then([&](const auto& sample) {
                       callable(sample.get());
                       numberOfSamples++;
                   })
                   .or_else([](auto& result) {
                       if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                       {
                           std::cerr << "Error receiving chunk!" << std::endl;
                       }
                   }))
        {
        }
        return numberOfSamples;
    }

    Future<FieldType> Get()
    {
        // If we call Get() twice shortly after each other, once the response of the first request has not yet
        // arrived, we have a problem
        bool requestSuccessfullySend{false};
        m_client.loan()
            .and_then([&](auto& request) {
                request.getRequestHeader().setSequenceId(m_sequenceId);
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
            return Future<FieldType>();
        }

        Promise<FieldType> promise;
        auto future = promise.get_future();
        // Typically you would e.g. use a worker pool here, for simplicity we use a plain thread
        std::thread(
            [&](Promise<FieldType>&& promise) {
                // Avoid race if MethodClient d'tor is called while this thread is still running
                std::lock_guard<iox::posix::mutex> guard(m_mutex);

                auto notificationVector = m_waitset.timedWait(iox::units::Duration::fromSeconds(5));

                for (auto& notification : notificationVector)
                {
                    if (notification->doesOriginateFrom(&m_client))
                    {
                        while (m_client.take().and_then([&](const auto& response) {
                            auto receivedSequenceId = response.getResponseHeader().getSequenceId();
                            if (receivedSequenceId == m_sequenceId)
                            {
                                FieldType result = *response;
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

    Future<FieldType> Set(const FieldType& value)
    {
        // If we call Set() twice shortly after each other, once the response of the first request has not yet
        // arrived, we have a problem
        bool requestSuccessfullySend{false};
        m_client.loan()
            .and_then([&](auto& request) {
                request.getRequestHeader().setSequenceId(m_sequenceId);
                request->emplace(value);
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
            return Future<FieldType>();
        }

        Promise<FieldType> promise;
        auto future = promise.get_future();
        // Typically you would e.g. use a worker pool here, for simplicity we use a plain thread
        std::thread(
            [&](Promise<FieldType>&& promise) {
                // Avoid race if MethodClient d'tor is called while this thread is still running
                std::lock_guard<iox::posix::mutex> guard(m_mutex);

                auto notificationVector = m_waitset.timedWait(iox::units::Duration::fromSeconds(5));

                for (auto& notification : notificationVector)
                {
                    if (notification->doesOriginateFrom(&m_client))
                    {
                        while (m_client.take().and_then([&](const auto& response) {
                            auto receivedSequenceId = response.getResponseHeader().getSequenceId();
                            if (receivedSequenceId == m_sequenceId)
                            {
                                FieldType result = *response;
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
    iox::popo::Subscriber<FieldType> m_subscriber;
    iox::popo::Client<iox::cxx::optional<FieldType>, FieldType> m_client;
    std::atomic<int64_t> m_sequenceId{0};
    iox::popo::WaitSet<> m_waitset;
    static constexpr bool isRecursive{true};
    iox::posix::mutex m_mutex{isRecursive};
};

} // namespace kom
} // namespace owl

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_SUBSCRIBER_HPP
