// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_TYPES_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_TYPES_HPP

#include "iceoryx_posh/internal/mepoo/chunk_management.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_pointer.hpp"

namespace iox
{
namespace popo
{
struct ChunkTuple
{
    ChunkTuple() = default;
    explicit ChunkTuple(const rp::RelativePointer<mepoo::ChunkManagement> chunk) noexcept;

    rp::BaseRelativePointer::id_t m_segmentId{rp::BaseRelativePointer::NULL_POINTER_ID};
    rp::BaseRelativePointer::offset_t m_chunkOffset{rp::BaseRelativePointer::NULL_POINTER_OFFSET};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CHUNK_QUEUE_TYPES_HPP
