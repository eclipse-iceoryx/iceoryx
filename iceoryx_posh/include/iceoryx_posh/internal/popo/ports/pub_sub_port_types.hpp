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
#ifndef IOX_POSH_POPO_PORTS_PUB_SUB_PORT_TYPES_HPP
#define IOX_POSH_POPO_PORTS_PUB_SUB_PORT_TYPES_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"

namespace iox
{
namespace popo
{
/// @todo iox-#1051 move definitions for publish subscribe communication here

using SubscriberChunkQueueData_t = ChunkQueueData<DefaultChunkQueueConfig, ThreadSafePolicy>;

using SubscriberChunkReceiverData_t =
    ChunkReceiverData<MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY, SubscriberChunkQueueData_t>;

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_PUB_SUB_PORT_TYPES_HPP
