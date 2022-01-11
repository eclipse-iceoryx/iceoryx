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

#ifndef IOX_HOOFS_CXX_UNIQUE_ID_HPP
#define IOX_HOOFS_CXX_UNIQUE_ID_HPP

#include "iceoryx_hoofs/cxx/newtype.hpp"

#include <atomic>

namespace iox
{
namespace cxx
{
/// @brief Unique IDs within a process starting with 1. Monotonic increasing IDs are
/// created with each call to the constructor. The IDs are copy/move constructible/assignable,
/// comparable, sortable and convertable to the underlying value type.
class UniqueId : public cxx::NewType<uint64_t,
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

    /// @brief the constructor creates an ID which is greater than the previous created ID
    UniqueId() noexcept;

  private:
    static std::atomic<value_type> g_IdCounter; // initialized in corresponding cpp file
};

} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_UNIQUE_ID_HPP
