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

#ifndef IOX_HOOFS_DESIGN_MOVE_AND_COPY_HELPER_INL
#define IOX_HOOFS_DESIGN_MOVE_AND_COPY_HELPER_INL

#include "iox/move_and_copy_helper.hpp"

namespace iox
{
template <MoveAndCopyOperations Opt>
template <typename T, typename V>
inline void MoveAndCopyHelper<Opt>::transfer(T& dest, V&& src) noexcept
{
    if constexpr (is_ctor)
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
inline void MoveAndCopyHelper<Opt>::create_new(T& dest, V&& src) noexcept
{
    if constexpr (is_move)
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
inline void MoveAndCopyHelper<Opt>::assign(T& dest, V&& src) noexcept
{
    if constexpr (is_move)
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
} // namespace iox
#endif
