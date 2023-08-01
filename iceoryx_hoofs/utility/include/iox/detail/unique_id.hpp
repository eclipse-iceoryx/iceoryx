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

#ifndef IOX_HOOFS_UTILITY_UNIQUE_ID_HPP
#define IOX_HOOFS_UTILITY_UNIQUE_ID_HPP

#include "iox/newtype.hpp"

#include <atomic>

namespace iox
{
/// @brief Unique IDs within a process starting with 1. Monotonic increasing IDs are
/// created with each call to the constructor. The IDs are copy/move constructible/assignable,
/// comparable, sortable and convertable to the underlying value type.
class UniqueId : public NewType<UniqueId,
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

    /// @brief the constructor creates an ID which is greater than the previous created ID
    UniqueId() noexcept;

  private:
    /// @NOLINTJUSTIFICATION only accessible by this class. the global variable is required to
    ///                      generate a unique id from it incrementing value
    /// @NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    static std::atomic<value_type> m_IdCounter; // initialized in corresponding cpp file
};

} // namespace iox

#endif // IOX_HOOFS_UTILITY_UNIQUE_ID_HPP
