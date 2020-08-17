// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"

namespace iox
{
namespace popo
{
namespace internal
{
static uint16_t uniqueRouDiId{0u};
static bool hasDefinedUniqueRouDiId{false};

void setUniqueRouDiId(const uint16_t id) noexcept
{
    if (hasDefinedUniqueRouDiId)
    {
        errorHandler(
            Error::kPOPO__TYPED_UNIQUE_ID_ROUDI_HAS_ALREADY_DEFINED_UNIQUE_ID, [] {}, ErrorLevel::MODERATE);
    }
    uniqueRouDiId = id;
    hasDefinedUniqueRouDiId = true;
}

void unsetUniqueRouDiId() noexcept
{
    hasDefinedUniqueRouDiId = false;
}

uint16_t getUniqueRouDiId() noexcept
{
    if (!hasDefinedUniqueRouDiId)
    {
        errorHandler(
            Error::kPOPO__TYPED_UNIQUE_ID_ROUDI_HAS_NO_DEFINED_UNIQUE_ID, [] {}, ErrorLevel::FATAL);
    }
    return uniqueRouDiId;
}

} // namespace internal
} // namespace popo
} // namespace iox
