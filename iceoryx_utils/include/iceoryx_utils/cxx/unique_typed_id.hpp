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
#ifndef IOX_UTILS_CXX_UNIQUE_ID_HPP
#define IOX_UTILS_CXX_UNIQUE_ID_HPP

#include <atomic>
#include <cstdint>
#include <limits>

namespace iox
{
namespace cxx
{
/// @brief Unique ID depending on type. If you would like to assign different
///         types consistent unique ids use this class. Every types gets gets its
///         own distinct set of ids starting with 0. If the types are the same the
///         ids are the same.
///
/// @code
///     struct MyClass {
///         // some members;
///         iox::cxx::UniqueTypedID<MyClass> id;
///     };
///
///     struct MySecondClass {
///         // some members;
///         iox::cxx::UniqueTypedID<MySecondClass> id;
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
template <typename T>
class UniqueTypedID
{
  public:
    /// @brief the constructor creates an id which is greater then the
    ///         previous created id
    UniqueTypedID() noexcept;

    /// @brief move constructs a new id and invalidates rhs by setting
    ///         the id to INVALID_ID
    UniqueTypedID(UniqueTypedID&& rhs) noexcept;

    /// @brief move assigns rhs and invalidates rhs by setting
    ///         the id to INVALID_ID
    UniqueTypedID& operator=(UniqueTypedID&& rhs) noexcept;

    UniqueTypedID(const UniqueTypedID& rhs) = default;
    UniqueTypedID& operator=(const UniqueTypedID& rhs) = default;
    ~UniqueTypedID() = default;

    /// @brief if two ids are equal return true, otherwise false
    bool operator==(const UniqueTypedID& rhs) const noexcept;

    /// @brief if two ids are not equal return true, otherwise false
    bool operator!=(const UniqueTypedID& rhs) const noexcept;

    /// @brief if the id of this is less then rhs return true, otherwise false
    bool operator<(const UniqueTypedID& rhs) const noexcept;

    /// @brief if the id of this is less or equal then rhs return true, otherwise false
    bool operator<=(const UniqueTypedID& rhs) const noexcept;

    /// @brief if the id of this is greater or equal then rhs return true, otherwise false
    bool operator>=(const UniqueTypedID& rhs) const noexcept;

    /// @brief if the id of this is greater then rhs return true, otherwise false
    bool operator>(const UniqueTypedID& rhs) const noexcept;

    /// @brief converts the id into a uint64_t which contains the actual
    ///         value of the id
    operator uint64_t() const noexcept;

    /// @brief returns the value of the id
    uint64_t getID() const noexcept;

    /// @brief constant which is used to signal an invalid id. used by the
    ///         move operations to notify the user that their object is no
    ///         longer valid.
    static constexpr uint64_t INVALID_ID = std::numeric_limits<uint64_t>::max();

  private:
    static std::atomic<uint64_t> globalIDCounter; // = 0u

    uint64_t m_id{INVALID_ID};
};

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/unique_typed_id.inl"

#endif
