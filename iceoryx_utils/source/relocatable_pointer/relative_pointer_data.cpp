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

#include "iceoryx_utils/internal/relocatable_pointer/relative_pointer_data.hpp"

namespace iox
{
namespace rp
{
static_assert(sizeof(RelativePointerData) <= 8U, "The RelativePointerData size must not exceed 64 bit!");
static_assert(std::is_trivially_copyable<RelativePointerData>::value,
              "The RelativePointerData must be trivially copyable!");

constexpr RelativePointerData::id_t RelativePointerData::ID_RANGE;
constexpr RelativePointerData::id_t RelativePointerData::NULL_POINTER_ID;
constexpr RelativePointerData::id_t RelativePointerData::MAX_VALID_ID;
constexpr RelativePointerData::offset_t RelativePointerData::OFFSET_RANGE;
constexpr RelativePointerData::offset_t RelativePointerData::NULL_POINTER_OFFSET;
constexpr RelativePointerData::offset_t RelativePointerData::MAX_VALID_OFFSET;
constexpr uint64_t RelativePointerData::LOGICAL_NULLPTR;

RelativePointerData::id_t RelativePointerData::id() const noexcept
{
    return static_cast<id_t>(m_idAndOffset & ID_RANGE);
}

RelativePointerData::offset_t RelativePointerData::offset() const noexcept
{
    return (m_idAndOffset >> 16) & OFFSET_RANGE;
}

void RelativePointerData::reset() noexcept
{
    this->m_idAndOffset = LOGICAL_NULLPTR;
}

bool RelativePointerData::isLogicalNullptr() const noexcept
{
    return m_idAndOffset == LOGICAL_NULLPTR;
}

} // namespace rp
} // namespace iox
