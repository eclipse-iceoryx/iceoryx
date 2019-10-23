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

#include "iceoryx_utils/cxx/helplets.hpp"

#include <atomic>
#include <cstdint>
#include <utility>

namespace iox
{
namespace mepoo
{
class MemPool;
struct ChunkInfo;

struct alignas(32) ChunkManagement
{
    using base_t = ChunkInfo;
    using referenceCounterBase_t = uint64_t;
    using referenceCounter_t = std::atomic<referenceCounterBase_t>;

    ChunkManagement(const cxx::not_null<base_t*> f_chunkInfo,
                    const cxx::not_null<MemPool*> f_mempool,
                    const cxx::not_null<MemPool*> f_chunkManagementPool)
        : m_chunkInfo(f_chunkInfo)
        , m_mempool(f_mempool)
        , m_chunkManagementPool(f_chunkManagementPool)
    {
    }

    base_t* m_chunkInfo;
    referenceCounter_t m_referenceCounter{1};
    MemPool* m_mempool;
    MemPool* m_chunkManagementPool;
};
} // namespace mepoo
} // namespace iox

