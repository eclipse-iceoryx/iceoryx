// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include "iceoryx_posh/mepoo/chunk_info.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/chunk_management.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_utils/internal/concurrent/sofi.hpp"

namespace iox
{
namespace popo
{
class DeliveryFiFo
{
  public:
    bool pop(mepoo::SharedChunk& chunk);
    bool push(mepoo::SharedChunk&& chunkIn, mepoo::SharedChunk& chunkOut);

    bool empty() const;
    bool resize(const uint32_t f_size);

  private:
    concurrent::SoFi<mepoo::ChunkManagement*, MAX_RECEIVER_QUEUE_SIZE> m_fifo;
};

} // namespace popo
} // namespace iox
