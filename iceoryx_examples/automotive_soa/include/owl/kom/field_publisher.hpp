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

#ifndef IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_HPP
#define IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/server.hpp"

#include "owl/types.hpp"

namespace owl
{
namespace kom
{
template <typename T>
class FieldPublisher
{
  public:
    using FieldType = T;
    static constexpr uint64_t HISTORY_CAPACITY{1U};

    FieldPublisher(const ServiceIdentifier& service,
                   const InstanceIdentifier& instance,
                   const FieldIdentifier& field,
                   const FieldType& fieldValue) noexcept;
    ~FieldPublisher() noexcept;

    FieldPublisher(const FieldPublisher&) = delete;
    FieldPublisher(FieldPublisher&&) = delete;
    FieldPublisher& operator=(const FieldPublisher&) = delete;
    FieldPublisher& operator=(FieldPublisher&&) = delete;

    bool Update(const FieldType& userField) noexcept;

    void RegisterGetHandler(const iox::cxx::function<void()>) noexcept;
    void RegisterSetHandler(const iox::cxx::function<void()>) noexcept;

  private:
    static void onRequestReceived(iox::popo::Server<iox::cxx::optional<FieldType>, FieldType>* server,
                                  FieldPublisher<FieldType>* self) noexcept;
    //! [FieldPublisher members]
    iox::popo::Publisher<FieldType> m_publisher;
    iox::popo::Server<iox::cxx::optional<FieldType>, FieldType> m_server;
    iox::popo::Listener m_listener;
    // latestValue is written concurrently by Listener and needs exclusive write access, alternatively a
    // concurrent::smart_lock could be used
    std::atomic<T> m_latestValue;
    //! [FieldPublisher members]
};
} // namespace kom
} // namespace owl

#include "owl/kom/field_publisher.inl"

#endif // IOX_EXAMPLES_AUTOMOTIVE_SOA_FIELD_PUBLISHER_HPP
