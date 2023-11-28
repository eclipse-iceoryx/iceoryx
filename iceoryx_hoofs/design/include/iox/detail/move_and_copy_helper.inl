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

#ifndef IOX_HOOFS_DESIGN_MOVE_AND_COPY_HELPER_INL
#define IOX_HOOFS_DESIGN_MOVE_AND_COPY_HELPER_INL

#include "iox/move_and_copy_helper.hpp"

namespace iox
{

/// @brief MoveAndCopyHelperBase is a template structure used to create or assign objects based on the provided
/// operation type (Opt). All of methods in this class are general for MoveAndCopyOperations. Any function that depends
/// on specific MoveAndCopyOperations should move to partial specialization region.
/// @tparam Opt The operation type that determines how objects are created or assigned.
template <MoveAndCopyOperations Opt>
class MoveAndCopyHelperBase
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

template <MoveAndCopyOperations Opt>
template <typename T, typename V>
inline void MoveAndCopyHelperBase<Opt>::transfer(T& dest, V&& src) noexcept
{
    if constexpr (is_ctor())
    {
        create_new(dest, std::forward<V>(src));
    }
    else
    {
        assign(dest, std::forward<V>(src));
    }
}

template <MoveAndCopyOperations Opt>
template <typename T, typename V>
inline void MoveAndCopyHelperBase<Opt>::ctor_create(T& dest, V&& src) noexcept
{
    if constexpr (is_move())
    {
        static_assert(std::is_rvalue_reference_v<decltype(src)>, "src should be rvalue reference");
        static_assert(std::is_convertible_v<V, T>, "src type is not convertible to dest type");
        new (&dest) T(std::forward<V>(src));
    }
    else
    {
        static_assert(std::is_lvalue_reference_v<decltype(src)>, "src should be lvalue reference");
        static_assert(std::is_const_v<std::remove_reference_t<decltype(src)>>, "src should have 'const' modifier");
        static_assert(std::is_convertible_v<V, T>, "src type is not convertible to dest type");
        new (&dest) T(src);
    }
}

template <MoveAndCopyOperations Opt>
template <typename T, typename V>
inline void MoveAndCopyHelperBase<Opt>::assignment_create(T& dest, V&& src) noexcept
{
    if constexpr (is_move())
    {
        static_assert(std::is_rvalue_reference_v<decltype(src)>, "src should be rvalue reference");
        dest = std::forward<V>(src);
    }
    else
    {
        static_assert(std::is_lvalue_reference_v<decltype(src)>, "src should be lvalue reference");
        static_assert(std::is_const_v<std::remove_reference_t<decltype(src)>>, "src should have 'const' modifier");
        dest = src;
    }
}

template <MoveAndCopyOperations Opt>
inline constexpr bool MoveAndCopyHelperBase<Opt>::is_ctor() noexcept
{
    return Opt == MoveAndCopyOperations::CopyConstructor || Opt == MoveAndCopyOperations::MoveConstructor;
}

template <MoveAndCopyOperations Opt>
inline constexpr bool MoveAndCopyHelperBase<Opt>::is_move() noexcept
{
    return Opt == MoveAndCopyOperations::MoveAssignment || Opt == MoveAndCopyOperations::MoveConstructor;
}

/// @brief The partial specialization region for MoveAndCopyHelper.
/* partial specialization */

template <>
struct MoveAndCopyHelper<MoveAndCopyOperations::CopyConstructor>
    : public MoveAndCopyHelperBase<MoveAndCopyOperations::CopyConstructor>
{
    template <typename T>
    static constexpr auto move_or_copy(const T& value) noexcept -> decltype(value)
    {
        return value;
    }

    template <typename Iterator>
    static constexpr auto move_or_copy_it(Iterator& it) noexcept -> decltype(*it)
    {
        return *it;
    }
};

template <>
struct MoveAndCopyHelper<MoveAndCopyOperations::CopyAssignment>
    : public MoveAndCopyHelperBase<MoveAndCopyOperations::CopyConstructor>
{
    /// @brief This function aims to simplified the constexpr if branches due to the 'transfer' parameters.
    /// @tparam T
    /// @param[in] value The target value which is going to transfer
    /// @return Const lvalue reference of value
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
    template <typename T>
    static inline constexpr auto move_or_copy(const T& value) noexcept -> decltype(value)
    {
        return value;
    }

    /// @brief The iterator as input version of move_or_copy
    /// @tparam Iterator The iterator type
    /// @param[in] it
    /// @return
    template <typename Iterator>
    static inline constexpr auto move_or_copy_it(Iterator& it) noexcept -> decltype(*it)
    {
        return *it;
    }
};

template <>
struct MoveAndCopyHelper<MoveAndCopyOperations::MoveConstructor>
    : public MoveAndCopyHelperBase<MoveAndCopyOperations::MoveConstructor>
{
    template <typename T>
    static inline constexpr auto move_or_copy(T&& value) noexcept -> decltype(std::move(std::forward<T>(value)))
    {
        return std::move(std::forward<T>(value));
    }

    template <typename Iterator>
    static inline constexpr auto move_or_copy_it(Iterator& it) noexcept -> decltype(std::move(*it))
    {
        return std::move(*it);
    }
};

template <>
struct MoveAndCopyHelper<MoveAndCopyOperations::MoveAssignment>
    : public MoveAndCopyHelperBase<MoveAndCopyOperations::MoveAssignment>
{
    template <typename T>
    static inline constexpr auto move_or_copy(T&& value) noexcept -> decltype(std::move(std::forward<T>(value)))
    {
        return std::move(std::forward<T>(value));
    }

    template <typename Iterator>
    static inline constexpr auto move_or_copy_it(Iterator& it) noexcept -> decltype(std::move(*it))
    {
        return std::move(*it);
    }
};

} // namespace iox
#endif
