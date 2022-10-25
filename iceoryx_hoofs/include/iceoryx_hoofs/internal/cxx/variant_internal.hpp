// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_VARIANT_INTERNAL_HPP
#define IOX_HOOFS_CXX_VARIANT_INTERNAL_HPP

#include "iceoryx_hoofs/cxx/requires.hpp"

#include <cstdint>
#include <type_traits>
#include <utility>

namespace iox
{
namespace cxx
{
template <uint64_t N>
struct in_place_index;

template <typename T>
struct in_place_type;
namespace internal
{
template <typename N>
struct is_in_place_index : std::false_type
{
};

template <uint64_t N>
struct is_in_place_index<in_place_index<N>> : std::true_type
{
};

template <typename T>
struct is_in_place_type : std::false_type
{
};

template <typename T>
struct is_in_place_type<in_place_type<T>> : std::true_type
{
};

using byte_t = uint8_t;
template <typename TypeToCheck, typename T, typename... Targs>
struct does_contain_type
{
    static constexpr bool value =
        std::is_same<TypeToCheck, T>::value || does_contain_type<TypeToCheck, Targs...>::value;
};

template <typename TypeToCheck, typename T>
struct does_contain_type<TypeToCheck, T>
{
    static constexpr bool value = std::is_same<TypeToCheck, T>::value;
};

template <uint64_t N, typename Type, typename T, typename... Targs>
struct get_index_of_type
{
    static constexpr uint64_t index = get_index_of_type<N + 1, Type, Targs...>::index;
};

template <uint64_t N, typename Type, typename... Targs>
struct get_index_of_type<N, Type, Type, Targs...>
{
    static constexpr uint64_t index = N;
};

template <uint64_t N, uint64_t Index, typename T, typename... Targs>
struct get_type_at_index
{
    using type = typename get_type_at_index<N + 1, Index, Targs...>::type;
};

template <uint64_t N, typename T, typename... Targs>
struct get_type_at_index<N, N, T, Targs...>
{
    using type = T;
};

template <uint64_t N, typename T, typename... Targs>
struct call_at_index
{
    static void destructor(const uint64_t index, byte_t* ptr) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<T*>(ptr)->~T();
        }
        else
        {
            call_at_index<N + 1, Targs...>::destructor(index, ptr);
        }
    }

    static void move(const uint64_t index, byte_t* source, byte_t* destination) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            *reinterpret_cast<T*>(destination) = std::move(*reinterpret_cast<T*>(source));
        }
        else
        {
            call_at_index<N + 1, Targs...>::move(index, source, destination);
        }
    }

    static void moveConstructor(const uint64_t index, byte_t* source, byte_t* destination) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            new (destination) T(std::move(*reinterpret_cast<T*>(source)));
        }
        else
        {
            call_at_index<N + 1, Targs...>::moveConstructor(index, source, destination);
        }
    }

    static void copy(const uint64_t index, const byte_t* source, byte_t* destination) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            *reinterpret_cast<T*>(destination) = *reinterpret_cast<const T*>(source);
        }
        else
        {
            call_at_index<N + 1, Targs...>::copy(index, source, destination);
        }
    }

    static void copyConstructor(const uint64_t index, const byte_t* source, byte_t* destination) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            new (destination) T(*reinterpret_cast<const T*>(source));
        }
        else
        {
            call_at_index<N + 1, Targs...>::copyConstructor(index, source, destination);
        }
    }

    static bool equality(const uint64_t index, const byte_t* lhs, const byte_t* rhs)
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter and encapsulated in this class
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return *reinterpret_cast<const T*>(lhs) == *reinterpret_cast<const T*>(rhs);
        }
        return call_at_index<N + 1, Targs...>::equality(index, lhs, rhs);
    }
};

template <uint64_t N, typename T>
struct call_at_index<N, T>
{
    // NOLINTJUSTIFICATION d'tor changes the data to which source is pointing to
    // NOLINTNEXTLINE(readability-non-const-parameter)
    static void destructor(const uint64_t index, byte_t* ptr) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<T*>(ptr)->~T();
        }
        else
        {
            ExpectsWithMsg(false, "Could not call destructor for variant element");
        }
    }

    // NOLINTJUSTIFICATION move c'tor changes the data to which source is pointing to
    // NOLINTNEXTLINE(readability-non-const-parameter)
    static void move(const uint64_t index, byte_t* source, byte_t* destination) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            *reinterpret_cast<T*>(destination) = std::move(*reinterpret_cast<T*>(source));
        }
        else
        {
            ExpectsWithMsg(false, "Could not call move assignment for variant element");
        }
    }

    // NOLINTJUSTIFICATION Both 'source' and 'destination' will be changed and can't be const
    // NOLINTNEXTLINE(readability-non-const-parameter)
    static void moveConstructor(const uint64_t index, byte_t* source, byte_t* destination) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            new (destination) T(std::move(*reinterpret_cast<T*>(source)));
        }
        else
        {
            ExpectsWithMsg(false, "Could not call move constructor for variant element");
        }
    }

    static void copy(const uint64_t index, const byte_t* source, byte_t* destination) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            *reinterpret_cast<T*>(destination) = *reinterpret_cast<const T*>(source);
        }
        else
        {
            ExpectsWithMsg(false, "Could not call copy assignment for variant element");
        }
    }

    // NOLINTJUSTIFICATION 'operator new()' needs non-const 'destination'
    // NOLINTNEXTLINE(readability-non-const-parameter)
    static void copyConstructor(const uint64_t index, const byte_t* source, byte_t* destination) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            new (destination) T(*reinterpret_cast<const T*>(source));
        }
        else
        {
            ExpectsWithMsg(false, "Could not call copy constructor for variant element");
        }
    }

    static bool equality(const uint64_t index, const byte_t* lhs, const byte_t* rhs) noexcept
    {
        if (N == index)
        {
            // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type safety ensured through template parameter and encapsulated in this class
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            return *reinterpret_cast<const T*>(lhs) == *reinterpret_cast<const T*>(rhs);
        }
        ExpectsWithMsg(false, "Could not call equality operator for variant element");
        return false;
    }
};

} // namespace internal
} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_VARIANT_INTERNAL_HPP
