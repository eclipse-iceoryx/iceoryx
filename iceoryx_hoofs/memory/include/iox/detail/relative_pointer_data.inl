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

#ifndef IOX_HOOFS_MEMORY_RELATIVE_POINTER_DATA_INL
#define IOX_HOOFS_MEMORY_RELATIVE_POINTER_DATA_INL

#include "iox/detail/relative_pointer_data.hpp"

namespace iox
{
// AXIVION Next Construct AutosarC++19_03-A12.1.2 : NSDMI with null value is more explicit
constexpr RelativePointerData::RelativePointerData(identifier_t id, offset_t offset) noexcept
    : m_idAndOffset(static_cast<uint64_t>(id) | (offset << ID_BIT_SIZE))
{
    if ((id > MAX_VALID_ID) || (offset > MAX_VALID_OFFSET))
    {
        m_idAndOffset = LOGICAL_NULLPTR;
    }
}
} // namespace iox

#endif // IOX_HOOFS_MEMORY_RELATIVE_POINTER_DATA_INL
