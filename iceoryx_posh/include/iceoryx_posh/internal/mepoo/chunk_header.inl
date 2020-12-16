// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_MEPOO_CHUNK_HEADER_INL
#define IOX_POSH_MEPOO_CHUNK_HEADER_INL

namespace iox
{
namespace mepoo
{
template <typename T>
T* ChunkHeader::customHeader() const noexcept
{
    // the CustomHeader is always located relative to "this" in this way
    return reinterpret_cast<T*>(reinterpret_cast<uint64_t>(this) + sizeof(ChunkHeader));
}
} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_CHUNK_HEADER_INL
