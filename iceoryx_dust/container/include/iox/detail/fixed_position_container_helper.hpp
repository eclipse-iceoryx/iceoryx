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

#ifndef IOX_DUST_CONTAINER_DETAIL_FIXED_POSITION_CONTAINER_HELPER_HPP
#define IOX_DUST_CONTAINER_DETAIL_FIXED_POSITION_CONTAINER_HELPER_HPP

#include <functional>
#include <utility>

namespace iox
{
namespace detail
{

enum class SpecialCreationOperations
{
    CopyConstructor,
    CopyAssignment,
    MoveConstructor,
    MoveAssignment,
};

/// @brief SpecialCreationHelper is a template structure used to create or assign objects based on the provided
/// operation type (Opt).
/// @tparam Opt The operation type that determines how objects are created or assigned.
template <SpecialCreationOperations Opt>
struct SpecialCreationHelper
{
    /// @brief Creates or assigns an object to 'dest' based on the specail operation type.
    /// @tparam T The type of the object to be created or assigned.
    /// @tparam V The type of the source object, kept as a universal reference to preserve its lvalue or rvalue nature.
    /// @param[out] dest The destination object where the new object is created or to which the source object is
    /// assigned.
    /// @param[in] src The source object, either for copy or move operations.
    template <typename T, typename V>
    static inline void create(T& dest, V&& src) noexcept
    {
        if constexpr (is_ctor())
        {
            ctor_create(dest, std::forward<V>(src));
        }
        else
        {
            assignment_create(dest, std::forward<V>(src));
        }
    }

    /// @brief Force to use constructor to create an object at the destination.
    /// @tparam T The type of the object to be constructed.
    /// @tparam V The type of the source object, used for move or copy construction.
    /// @param[out] dest The destination object where the new object is constructed.
    /// @param[in] src The source object, either for move or copy construction.
    template <typename T, typename V>
    static inline void ctor_create(T& dest, V&& src) noexcept
    {
        if constexpr (is_move())
        {
            static_assert(std::is_rvalue_reference_v<decltype(src)>, "src should be rvalue reference");
            static_assert(std::is_convertible_v<V, T>, "src type is not convertible to dest type");
            new (&dest) T(std::forward<V>(src));
        }
        else
        {
            // @todo iox-2052: enable this when move_or_copy_it / move_or_copy works
            // static_assert(std::is_lvalue_reference_v<decltype(src.get())>, "src should be lvalue reference");
            // static_assert(std::is_const_v<std::remove_reference_t<decltype(src.get())>>, "src should has const
            // modifier"); static_assert(std::is_convertible_v<V, T>, "Source type is not convertible to destination
            // type"); new (&dest) T(src.get());

            static_assert(std::is_lvalue_reference_v<decltype(src)>, "src should be lvalue reference");
            static_assert(std::is_const_v<std::remove_reference_t<decltype(src)>>, "src should has 'const' modifier");
            static_assert(std::is_convertible_v<V, T>, "src type is not convertible to dest type");
            new (&dest) T(src);
        }
    }

    /// @brief Force to use assignment to assign an object to the destination.
    /// @tparam T The type of the destination object.
    /// @tparam V The type of the source object, used for move or copy assignment.
    /// @param dest The destination object where the source object is assigned.
    /// @param src The source object, either for move or copy assignment.
    template <typename T, typename V>
    static inline void assignment_create(T& dest, V&& src) noexcept
    {
        if constexpr (is_move())
        {
            static_assert(std::is_rvalue_reference_v<decltype(src)>, "src should be rvalue reference");
            dest = std::forward<V>(src);
        }
        else
        {
            // @todo iox-2052: enable this when move_or_copy_it / move_or_copy works
            // static_assert(std::is_lvalue_reference_v<decltype(src.get())>, "src should be lvalue reference");
            // static_assert(std::is_const_v<std::remove_reference_t<decltype(src.get())>>, "src should has const
            // modifier"); dest = src.get();

            static_assert(std::is_lvalue_reference_v<decltype(src)>, "src should be lvalue reference");
            static_assert(std::is_const_v<std::remove_reference_t<decltype(src)>>, "src should has 'const' modifier");
            dest = src;
        }
    }

    /// @brief Checks if the current special operation is a constructor call.
    /// @return True if the operation is a copy or move constructor, false otherwise.
    static constexpr bool is_ctor() noexcept
    {
        return Opt == SpecialCreationOperations::CopyConstructor || Opt == SpecialCreationOperations::MoveConstructor;
    }

    /// @brief Checks if the current special operation is a move operation.
    /// @return True if the operation is a move constructor or move assignment, false otherwise.
    static constexpr bool is_move() noexcept
    {
        return Opt == SpecialCreationOperations::MoveAssignment || Opt == SpecialCreationOperations::MoveConstructor;
    }

    // template <typename T>
    // static inline constexpr auto move_or_copy(T&& value) noexcept
    // {
    //     if constexpr (is_move())
    //     {
    //         // FIXME: See https://godbolt.org/z/hz9Kbb8nr.
    //         return std::move(std::forward<T>(value));
    //     }
    //     else
    //     {
    //         return std::cref(std::forward<T>(value));
    //     }
    // }

    // template <typename Iterator>
    // static inline constexpr auto move_or_copy_it(Iterator& it) noexcept
    // {
    //     if constexpr (is_move())
    //     {
    //         // FIXME: Twice time movector will be called.
    //         return std::move(*it);
    //     }
    //     else
    //     {
    //         // std::cref fix twice time copyCtor problem?
    //         // See https://godbolt.org/z/W41ndvhh5
    //         return std::cref(*it);
    //     }
    // }
};
} // namespace detail
} // namespace iox
#endif
