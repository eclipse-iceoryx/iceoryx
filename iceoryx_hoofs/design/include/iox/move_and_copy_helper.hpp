// Copyright (c) 2023 by Dennis Liu <dennis48161025@gmail.com>. All rights reserved. reserved.
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

#ifndef IOX_HOOFS_DESIGN_MOVE_AND_COPY_HELPER_HPP
#define IOX_HOOFS_DESIGN_MOVE_AND_COPY_HELPER_HPP

#include <functional>
#include <utility>

namespace iox
{

enum class MoveAndCopyOperations
{
    CopyConstructor,
    CopyAssignment,
    MoveConstructor,
    MoveAssignment,
};

/// @brief MoveAndCopyHelper is a template structure used to create or assign objects based on the provided
/// operation type (Opt).
/// @tparam Opt The operation type that determines how objects are created or assigned.
template <MoveAndCopyOperations Opt>
class MoveAndCopyHelper final
{
  public:
    /// @brief Creates or assigns an object to 'dest' based on the specail operation type.
    /// @tparam T The type of the object to be created or assigned.
    /// @tparam V The type of the source object, kept as a universal reference to preserve its lvalue or rvalue nature.
    /// @param[out] dest The destination object where the new object is created or to which the source object is
    /// assigned.
    /// @param[in] src The source object, either for copy or move operations.
    template <typename T, typename V>
    static void transfer(T& dest, V&& src) noexcept;

    /// @brief Force to use constructor to create an object at the destination.
    /// @tparam T The type of the object to be constructed.
    /// @tparam V The type of the source object, used for move or copy construction.
    /// @param[out] dest The destination object where the new object is constructed.
    /// @param[in] src The source object, either for move or copy construction.
    template <typename T, typename V>
    static void ctor_create(T& dest, V&& src) noexcept;

    /// @brief Force to use assignment to assign an object to the destination.
    /// @tparam T The type of the destination object.
    /// @tparam V The type of the source object, used for move or copy assignment.
    /// @param dest The destination object where the source object is assigned.
    /// @param src The source object, either for move or copy assignment.
    template <typename T, typename V>
    static void assignment_create(T& dest, V&& src) noexcept;

    /// @brief Checks if the current special operation is a constructor call.
    /// @return True if the operation is a copy or move constructor, false otherwise.
    static constexpr bool is_ctor() noexcept;

    /// @brief Checks if the current special operation is a move operation.
    /// @return True if the operation is a move constructor or move assignment, false otherwise.
    static constexpr bool is_move() noexcept;
};

} // namespace iox

#include "iox/detail/move_and_copy_helper.inl"

#endif
