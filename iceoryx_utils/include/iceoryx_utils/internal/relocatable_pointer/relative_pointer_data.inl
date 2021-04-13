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

#ifndef IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_POINTER_DATA_INL
#define IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_POINTER_DATA_INL

namespace iox
{
namespace rp
{
constexpr RelativePointerData::RelativePointerData(id_t id, offset_t offset) noexcept
    : m_idAndOffset(static_cast<uint64_t>(id) | (offset << 16U))
{
    /// @todo iox-#605 if PointerRepository::registerPtr checks for max id <= MAX_ID and max size <= MAX_OFFSET, this
    /// could be removed
    cxx::Ensures(id <= MAX_VALID_ID && "id must not exceed MAX_VALID_ID!");
    cxx::Ensures(offset <= MAX_VALID_OFFSET && "offset must not exceed MAX_VALIDOFFSET!");
}
} // namespace rp
} // namespace iox

#endif // IOX_UTILS_RELOCATABLE_POINTER_RELATIVE_POINTER_DATA_INL
