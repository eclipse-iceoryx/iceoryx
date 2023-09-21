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

#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

namespace iox
{
namespace popo
{
std::atomic<uint16_t> UniquePortId::uniqueRouDiId{roudi::DEFAULT_UNIQUE_ROUDI_ID};

// start with 1 to prevent accidentally generating an invalid ID when unique roudi ID is 0
std::atomic<UniquePortId::value_type> UniquePortId::globalIDCounter{1U};

UniquePortId::UniquePortId() noexcept
    : ThisType(newtype::internal::ProtectedConstructor,
               (static_cast<UniquePortId::value_type>(getUniqueRouDiId()) << UNIQUE_ID_BIT_LENGTH)
                   + ((globalIDCounter.fetch_add(1u, std::memory_order_relaxed) << ROUDI_ID_BIT_LENGTH)
                      >> ROUDI_ID_BIT_LENGTH))
{
    UniquePortId::finalizeSetUniqueRouDiId();

    if (globalIDCounter.load() >= (static_cast<UniquePortId::value_type>(1u) << UNIQUE_ID_BIT_LENGTH))
    {
        errorHandler(PoshError::POPO__TYPED_UNIQUE_ID_OVERFLOW, ErrorLevel::FATAL);
    }
}

UniquePortId::UniquePortId(InvalidPortId_t) noexcept
    /// we have to cast INVALID_UNIQUE_ID with static_cast<value_type> otherwise it will not link
    /// with gcc-7.x - gcc-10.x. Who knows why?!
    : ThisType(newtype::internal::ProtectedConstructor, static_cast<UniquePortId::value_type>(INVALID_UNIQUE_ID))
{
    // finalizeSetUniqueRouDiId intentionally not called since the InvalidPortId does not have a unique RouDi ID anyway
}

bool UniquePortId::isValid() const noexcept
{
    return UniquePortId(InvalidPortId) != *this;
}

void UniquePortId::setUniqueRouDiId(const uint16_t id) noexcept
{
    if (finalizeSetUniqueRouDiId())
    {
        errorHandler(PoshError::POPO__TYPED_UNIQUE_ID_ROUDI_HAS_ALREADY_DEFINED_CUSTOM_UNIQUE_ID, ErrorLevel::SEVERE);
    }
    uniqueRouDiId.store(id, std::memory_order_relaxed);
}

void UniquePortId::rouDiEnvOverrideUniqueRouDiId(const uint16_t id) noexcept
{
    uniqueRouDiId.store(id, std::memory_order_relaxed);
}

bool UniquePortId::finalizeSetUniqueRouDiId() noexcept
{
    static bool finalized{false};
    auto oldFinalized = finalized;
    finalized = true;
    return oldFinalized;
}

uint16_t UniquePortId::getUniqueRouDiId() noexcept
{
    return uniqueRouDiId.load(std::memory_order_relaxed);
}

} // namespace popo
} // namespace iox
