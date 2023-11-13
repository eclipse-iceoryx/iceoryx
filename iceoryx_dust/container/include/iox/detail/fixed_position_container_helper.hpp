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

#include <utility>

namespace iox
{
namespace detail
{
template <bool IsMove>
struct AssignmentHelper;

template <>
struct AssignmentHelper<true>
{
    template <typename T>
    static void assign(T& dest, T&& src)
    {
        dest = std::forward<T>(src);
    }
};

template <>
struct AssignmentHelper<false>
{
    template <typename T>
    static void assign(T& dest, const T& src)
    {
        dest = src;
    }
};

template <bool IsMove>
struct CtorHelper;

template <>
struct CtorHelper<true>
{
    template <typename T>
    static void construct(T& dest, T&& src)
    {
        new (&dest) T(std::forward<T>(src));
    }
};

template <>
struct CtorHelper<false>
{
    template <typename T>
    static void construct(T& dest, const T& src)
    {
        new (&dest) T(src);
    }
};

template <bool IsMove>
struct MoveHelper;

template <>
struct MoveHelper<true>
{
    template <typename Iterator>
    static auto move_or_copy(Iterator& it) -> decltype(std::move(*it))
    {
        return std::move(*it);
    }
};

template <>
struct MoveHelper<false>
{
    template <typename Iterator>
    static auto move_or_copy(Iterator& it) -> decltype(*it)
    {
        return *it;
    }
};

#endif
}
}
