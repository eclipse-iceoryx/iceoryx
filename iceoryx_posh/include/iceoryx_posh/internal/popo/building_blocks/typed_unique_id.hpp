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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_TYPED_UNIQUE_ID_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_TYPED_UNIQUE_ID_HPP

#include "iceoryx_utils/cxx/newtype.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

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
///         If you would like to reset the unique id you have to call
///         unsetUniqueRouDiId first.
/// @param[in] id the unique id which you would like to set
void setUniqueRouDiId(const uint16_t id) noexcept;

/// @brief Sets the RouDi id to an undefined state
void unsetUniqueRouDiId() noexcept;

/// @brief returns the unique roudi id
/// @return value of the unique roudi id
uint16_t getUniqueRouDiId() noexcept;
} // namespace internal

/// @brief Struct to signal the constructor to create an invalid id
struct CreateInvalidId_t
{
};
constexpr CreateInvalidId_t CreateInvalidId = CreateInvalidId_t();

/// @brief Unique ID depending on a type. If you would like to assign different
///         types consistent unique ids use this class. Every types gets its
///         own distinct set of ids starting with 0. If the types are the same the
///         ids are the same.
///
/// @code
///     struct MyClass {
///         // some members;
///         iox::port::TypedUniqueId<MyClass> id;
///     };
///
///     struct MySecondClass {
///         // some members;
///         iox::port::TypedUniqueId<MySecondClass> id;
///     };
//
///     std::vector<MyClass> myClassVector;
///     std::vector<MyClass> mySecondClassVector;
///
///     uint64_t AddClass() {
///         myClassVector.emplace_back();
///         return myClassVector.back().id.getID();
///     }
///
///     uint64_t AddSecondClass() {
///         mySecondClassVector.emplace_back();
///         // we use the uint64_t conversion
///         return mySecondClassVector.back().id;
///     }
///
///     void RemoveClass(const uint64_t id) {
///         auto iter = std::find_if(myClassVector.begin(), myClassVector.end(), [&](MyClass & c){ return c.id == id;});
///         if ( iter != myClassVector.end() ) myClassVector.erase(iter);
///     }
///
///     uint64_t id = AddClass();
///     RemoveClass(id);
///
///     // it can be that id == id2 since the id is unique per type
///     uint64_t id2 = AddSecondClass();
/// @endcode
/// @param[in] T type for which the unique ids should be generated
///
template <typename T>
class TypedUniqueId : public cxx::NewType<uint64_t,
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
    TypedUniqueId() noexcept;

    /// @brief constructor which creates an invalid id
    TypedUniqueId(CreateInvalidId_t) noexcept;

    bool isValid() const noexcept;

  private:
    static constexpr uint64_t INVALID_UNIQUE_ID = 0u;
    static constexpr uint64_t ROUDI_ID_BIT_LENGTH = 16u;
    static constexpr uint64_t UNIQUE_ID_BIT_LENGTH = 48u;
    static std::atomic<uint64_t> globalIDCounter; // = 0u
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.inl"

#endif
