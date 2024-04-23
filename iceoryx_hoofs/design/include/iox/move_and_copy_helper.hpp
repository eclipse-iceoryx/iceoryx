// Copyright (c) 2023 by Dennis Liu <dennis48161025@gmail.com>. All rights reserved.
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

#include <cstdint>
#include <functional>
#include <utility>

namespace iox
{

enum class MoveAndCopyOperations : uint8_t
{
    CopyConstructor,
    CopyAssignment,
    MoveConstructor,
    MoveAssignment,
};

/// @brief MoveAndCopyHelper is a template structure used to create or assign objects based on the provided
/// operation type (Opt). All of methods in this class are general for MoveAndCopyOperations. Any function that depends
/// on specific MoveAndCopyOperations should move to partial specialization region.
/// @tparam Opt The operation type that determines how objects are created or assigned.
template <MoveAndCopyOperations Opt>
class MoveAndCopyHelper
{
  public:
    static constexpr bool is_ctor =
        Opt == MoveAndCopyOperations::CopyConstructor || Opt == MoveAndCopyOperations::MoveConstructor;
    static constexpr bool is_move =
        Opt == MoveAndCopyOperations::MoveAssignment || Opt == MoveAndCopyOperations::MoveConstructor;

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
    static void create_new(T& dest, V&& src) noexcept;

    /// @brief Force to use assignment to assign an object to the destination.
    /// @tparam T The type of the destination object.
    /// @tparam V The type of the source object, used for move or copy assignment.
    /// @param dest The destination object where the source object is assigned.
    /// @param src The source object, either for move or copy assignment.
    template <typename T, typename V>
    static void assign(T& dest, V&& src) noexcept;

    /// @brief Moves a value if the condition specified by 'enable_if_t' is met.
    /// @details This overload of 'move_or_copy' is enabled when 'Opt' is either 'MoveAssignment' or 'MoveConstructor'.
    ///          It moves the given value, providing an efficient way to transfer resources.
    /// @tparam T The type of the value to be moved.
    /// @tparam enable_if A SFINAE condition that enables this overload for move operations.
    /// @param[in] src The source value to be moved.
    /// @return A reference to the moved value.
    /// @note This overload is selected when the template parameter 'Opt' indicates a move operation.
    /// @note We use std::remove_reference_t<T>&& as return type for 'T' might be a lvalue reference (for struct)
    /// @example
    /// An example of simplifying the usage of 'if constexpr' branches in a loop.
    /// The original code:
    /// \code{.cpp}
    /// for (uint64_t i = 0; i < rhs.size(); ++i)
    /// {
    ///     if constexpr (is_move)
    ///     {
    ///         Helper::transfer(m_data[i], std::move(rhs.data[i]));
    ///     }
    ///     else
    ///     {
    ///         Helper::transfer(m_data[i], rhs.data[i]);
    ///     }
    /// }
    /// \endcode
    /// can be simplified to:
    /// \code{.cpp}
    /// for (uint64_t i = 0; i < rhs.size(); ++i)
    /// {
    ///     Helper::transfer(m_data[i], Helper::move_or_copy(rhs.data[i]));
    /// }
    /// \endcode
    template <typename T, typename = std::enable_if_t<is_move, T>>
    static inline std::remove_reference_t<T>&& move_or_copy(T&& src) noexcept
    {
        static_assert(is_move, "is_move should be true");
        return std::move(std::forward<T>(src));
    }

    /// @brief Copies a value if the condition specified by 'enable_if_t' is not met for move operations.
    /// @details This overload of 'move_or_copy' is enabled for cases other than 'MoveAssignment' or 'MoveConstructor'.
    ///          It simply copies the given value.
    /// @tparam T The type of the value to be copied.
    /// @tparam enable_if A SFINAE condition that enables this overload for copy operations.
    /// @param[in] src The source value to be copied.
    /// @return A const reference to the copied value.
    /// @note This overload is selected when the template parameter 'Opt' does not indicate a move operation.
    template <typename T, typename = std::enable_if_t<!is_move, T>>
    static inline const T& move_or_copy(const T& src) noexcept
    {
        static_assert(!is_move, "is_move should be false");
        return src;
    }

    /// @brief Moves a value pointed by an iterator if the condition specified by 'enable_if_t' is met.
    /// @details This overload of 'move_or_copy_it' is enabled when 'Opt' is either 'MoveAssignment' or
    /// 'MoveConstructor'.
    ///          It moves the value pointed by the given iterator.
    /// @tparam Iterator The type of the iterator.
    /// @tparam enable_if A SFINAE condition that enables this overload for move operations.
    /// @param[in] it The iterator pointing to the value to be moved.
    /// @return A moved value from the iterator's pointee.
    /// @note This overload is selected for move operations based on 'Opt'.
    /// @note If the decltype syntax missed, move ctor will be called twice
    template <typename Iterator, typename = std::enable_if_t<is_move, Iterator>>
    static inline auto move_or_copy_it(Iterator& it) noexcept -> std::remove_reference_t<decltype(*it)>&&
    {
        static_assert(is_move, "is_move should be true");
        return std::move(*it);
    }

    /// @brief Accesses a value pointed by an iterator if the condition specified by 'enable_if_t' is not met for move
    /// operations.
    /// @details This overload of 'move_or_copy_it' is enabled for cases other than 'MoveAssignment' or
    /// 'MoveConstructor'.
    ///          It provides direct access to the value pointed by the iterator.
    /// @tparam Iterator The type of the iterator.
    /// @tparam enable_if A SFINAE condition that enables this overload for direct access operations.
    /// @param[in] it The iterator pointing to the value to be accessed.
    /// @return A direct access to the value from the iterator's pointee.
    /// @note This overload is selected for direct access operations when 'Opt' does not indicate a move operation.
    /// @note If the decltype syntax missed, move assignment will be called twice
    template <typename Iterator, typename = std::enable_if_t<!is_move, Iterator>>
    static inline auto move_or_copy_it(Iterator& it) noexcept -> const decltype(*it)&
    {
        static_assert(!is_move, "is_move should be false");
        return *it;
    }
};

} // namespace iox

#include "iox/detail/move_and_copy_helper.inl"

#endif
