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
#ifndef IOX_UTILS_CXX_TYPED_UNIQUE_ID_HPP
#define IOX_UTILS_CXX_TYPED_UNIQUE_ID_HPP

#include "iceoryx_utils/cxx/newtype.hpp"

#include <atomic>
#include <cstdint>
#include <limits>

namespace iox
{
namespace cxx
{
/// @brief Unique ID depending on type. If you would like to assign different
///         types consistent unique ids use this class. Every types gets its
///         own distinct set of ids starting with 0. If the types are the same the
///         ids are the same.
///
/// @code
///     struct MyClass {
///         // some members;
///         iox::cxx::TypedUniqueId<MyClass> id;
///     };
///
///     struct MySecondClass {
///         // some members;
///         iox::cxx::TypedUniqueId<MySecondClass> id;
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
class TypedUniqueId : public NewType<uint64_t,
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

    /// @brief the constructor creates an id which is greater than the
    ///         previous created id
    TypedUniqueId() noexcept;

  private:
    static std::atomic<uint64_t> globalIDCounter; // = 0u
};

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/typed_unique_id.inl"

#endif
