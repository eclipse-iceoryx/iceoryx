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
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/atomic.hpp"

namespace iox
{
namespace popo
{
// start with 1 to prevent accidentally generating an invalid ID when unique roudi ID is 0
concurrent::Atomic<UniquePortId::value_type> UniquePortId::globalIDCounter{1U};

UniquePortId::UniquePortId(const roudi::UniqueRouDiId uniqueRouDiId) noexcept
    : ThisType(newtype::internal::ProtectedConstructor,
               (static_cast<UniquePortId::value_type>(static_cast<roudi::UniqueRouDiId::value_type>(uniqueRouDiId))
                << UNIQUE_ID_BIT_LENGTH)
                   + ((globalIDCounter.fetch_add(1u, std::memory_order_relaxed) << ROUDI_ID_BIT_LENGTH)
                      >> ROUDI_ID_BIT_LENGTH))
{
    if (globalIDCounter.load() >= (static_cast<UniquePortId::value_type>(1u) << UNIQUE_ID_BIT_LENGTH))
    {
        IOX_REPORT_FATAL(PoshError::POPO__TYPED_UNIQUE_ID_OVERFLOW);
    }
}

UniquePortId::UniquePortId(InvalidPortId_t) noexcept
    /// we have to cast INVALID_UNIQUE_ID with static_cast<value_type> otherwise it will not link
    /// with gcc-7.x - gcc-10.x. Who knows why?!
    : ThisType(newtype::internal::ProtectedConstructor, static_cast<UniquePortId::value_type>(INVALID_UNIQUE_ID))
{
}

bool UniquePortId::isValid() const noexcept
{
    return UniquePortId(InvalidPortId) != *this;
}

} // namespace popo
} // namespace iox
