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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_TYPED_UNIQUE_ID_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_TYPED_UNIQUE_ID_HPP

#include "iceoryx_hoofs/cxx/newtype.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"

#include <atomic>
#include <cstdint>
#include <limits>

namespace iox
{
namespace popo
{
namespace internal
{
/// @brief Has to be set on roudi startup so that a unique roudi id is set
///         for all newly generated unique ids. If you call it when a unique
///         id is already set an error is generated in the errorHandler.
/// @param[in] id the unique id which you would like to set
void setUniqueRouDiId(const uint16_t id) noexcept;

/// @brief This finalizes setting the unique roudi id and is used by setUniqueRouDiId
///        to check whether the id was already set or UniquePortId were created before
///        calling setUniqueRouDiId
/// @return true if setUniqueRouDiId was already called or a non-invalid UniquePortId
///         was created, otherwise false
bool finalizeSetUniqueRouDiId() noexcept;

/// @brief returns the unique roudi id
/// @return value of the unique roudi id
uint16_t getUniqueRouDiId() noexcept;
} // namespace internal

/// @brief Struct to signal the constructor to create an invalid id
struct InvalidPortId_t
{
};
constexpr InvalidPortId_t InvalidPortId = InvalidPortId_t();

/// @brief Unique ID depending on a type. If you would like to assign different
///         types consistent unique ids use this class. Every types gets its
///         own distinct set of ids starting with 0. If the types are the same the
///         ids are the same.
class UniquePortId : public cxx::NewType<uint64_t,
                                         cxx::newtype::ProtectedConstructByValueCopy,
                                         cxx::newtype::Comparable,
                                         cxx::newtype::Sortable,
                                         cxx::newtype::Convertable,
                                         cxx::newtype::CopyConstructable,
                                         cxx::newtype::MoveConstructable,
                                         cxx::newtype::CopyAssignable,
                                         cxx::newtype::MoveAssignable>
{
  public:
    using ThisType::ThisType;

    /// @brief the constructor creates an id which is greater than the
    ///         previous created id
    UniquePortId() noexcept;

    /// @brief constructor which creates an invalid id
    UniquePortId(InvalidPortId_t) noexcept;

    bool isValid() const noexcept;

  private:
    static constexpr ThisType::value_type INVALID_UNIQUE_ID = 0u;
    static constexpr ThisType::value_type ROUDI_ID_BIT_LENGTH = 16u;
    static constexpr ThisType::value_type UNIQUE_ID_BIT_LENGTH = 48u;
    static std::atomic<ThisType::value_type> globalIDCounter; // initialized in cpp file
};

} // namespace popo
} // namespace iox

#endif
