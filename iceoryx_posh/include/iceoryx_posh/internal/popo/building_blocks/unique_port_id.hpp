// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 - 2023 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_UNIQUE_PORT_ID_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_UNIQUE_PORT_ID_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/atomic.hpp"
#include "iox/newtype.hpp"

#include <cstdint>

namespace iox
{
namespace roudi_env
{
class RouDiEnv;
}

namespace popo
{
/// @brief Struct to signal the constructor to create an invalid id
struct InvalidPortId_t
{
};
constexpr InvalidPortId_t InvalidPortId = InvalidPortId_t();

/// @brief A counter which is monotonically advancing with each newly created instance of UniquePortId. Additionally
///        it contains a unique RouDi id to be able to differentiate the sample sources in a multi publisher multi
///        subscriber pattern where samples are exchanged over network via a third party middleware.
///        The unique RouDi id must be set manually when RouDi is started and it must be ensured to be unique for a
///        given instance for this feature to be used to its full extend.
class UniquePortId : public NewType<UniquePortId,
                                    uint64_t,
                                    newtype::ProtectedConstructByValueCopy,
                                    newtype::Comparable,
                                    newtype::Sortable,
                                    newtype::Convertable,
                                    newtype::CopyConstructable,
                                    newtype::MoveConstructable,
                                    newtype::CopyAssignable,
                                    newtype::MoveAssignable>
{
  public:
    using ThisType::ThisType;

    /// @brief The constructor creates an id which is greater than the previous created id
    /// @param[in] uniqueRouDiId to tie the unique port id to
    UniquePortId(const roudi::UniqueRouDiId uniqueRouDiId) noexcept;

    /// @brief Constructor which creates an invalid id
    UniquePortId(InvalidPortId_t) noexcept;

    /// @brief Indicates whether the object contains an invalid port id
    /// @return true if a valid id is present, false otherwise
    bool isValid() const noexcept;

  private:
    // NOTE must be 'delete' instead of just leaving it out else the 'gMocks' will create compile errors
    UniquePortId() noexcept = delete;

  private:
    static constexpr ThisType::value_type INVALID_UNIQUE_ID = 0u;
    static constexpr ThisType::value_type ROUDI_ID_BIT_LENGTH = 16u;
    static constexpr ThisType::value_type UNIQUE_ID_BIT_LENGTH = 48u;
    static concurrent::Atomic<ThisType::value_type> globalIDCounter; // initialized in cpp file
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_UNIQUE_PORT_ID_HPP
