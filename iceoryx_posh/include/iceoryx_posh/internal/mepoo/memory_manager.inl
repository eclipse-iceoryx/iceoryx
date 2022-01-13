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

#ifndef IOX_POSH_MEPOO_MEMORY_MANAGER_INL
#define IOX_POSH_MEPOO_MEMORY_MANAGER_INL

namespace iox
{
namespace mepoo
{
inline constexpr const char* asStringLiteral(const MemoryManager::Error value) noexcept
{
    switch (value)
    {
    case MemoryManager::Error::NO_MEMPOOLS_AVAILABLE:
        return "MemoryManager::Error::NO_MEMPOOLS_AVAILABLE";
    case MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE:
        return "MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE";
    case MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS:
        return "MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS";
    }

    return "[Undefined MemoryManager::Error]";
}

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_MEMORY_MANAGER_INL
