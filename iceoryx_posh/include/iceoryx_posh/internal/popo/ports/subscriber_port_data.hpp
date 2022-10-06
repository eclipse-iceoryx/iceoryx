// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_PORTS_SUBSCRIBER_PORT_DATA_HPP
#define IOX_POSH_POPO_PORTS_SUBSCRIBER_PORT_DATA_HPP

#include "iceoryx_hoofs/cxx/variant_queue.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/pub_sub_port_types.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"

#include <atomic>

namespace iox
{
namespace popo
{
struct SubscriberOptions;

struct SubscriberPortData : public BasePortData
{
    SubscriberPortData(const capro::ServiceDescription& serviceDescription,
                       const RuntimeName_t& runtimeName,
                       cxx::VariantQueueTypes queueType,
                       const SubscriberOptions& subscriberOptions,
                       const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept;

    /// @todo iox-#1051 remove these aliases here and only depend on pub_sub_port_types.hpp
    ///       (move relevant types and constants there)
    using ChunkQueueData_t = iox::popo::SubscriberChunkQueueData_t;
    using ChunkReceiverData_t = iox::popo::SubscriberChunkReceiverData_t;

    ChunkReceiverData_t m_chunkReceiverData;

    SubscriberOptions m_options;

    std::atomic_bool m_subscribeRequested{false};
    std::atomic<SubscribeState> m_subscriptionState{SubscribeState::NOT_SUBSCRIBED};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_SUBSCRIBER_PORT_DATA_HPP
