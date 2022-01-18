// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

namespace iox
{
namespace popo
{
namespace internal
{
static uint16_t uniqueRouDiId{DEFAULT_UNIQUE_ROUDI_ID};

void setUniqueRouDiId(const uint16_t id) noexcept
{
    if (finalizeSetUniqueRouDiId())
    {
        errorHandler(
            Error::kPOPO__TYPED_UNIQUE_ID_ROUDI_HAS_ALREADY_DEFINED_CUSTOM_UNIQUE_ID, nullptr, ErrorLevel::SEVERE);
    }
    uniqueRouDiId = id;
}

bool finalizeSetUniqueRouDiId() noexcept
{
    static bool finalized{false};
    auto oldFinalized = finalized;
    finalized = true;
    return oldFinalized;
}

uint16_t getUniqueRouDiId() noexcept
{
    return uniqueRouDiId;
}

} // namespace internal
} // namespace popo
} // namespace iox
