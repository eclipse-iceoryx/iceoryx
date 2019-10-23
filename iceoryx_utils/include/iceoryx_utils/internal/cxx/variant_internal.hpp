// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include <assert.h>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace iox
{
namespace cxx
{
namespace internal
{
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
    static void destructor(const uint64_t index, byte_t* ptr)
    {
        if (N == index)
        {
            reinterpret_cast<T*>(ptr)->~T();
        }
        else
        {
            call_at_index<N + 1, Targs...>::destructor(index, ptr);
        }
    }

    static void move(const uint64_t index, byte_t* source, byte_t* destination)
    {
        if (N == index)
        {
            *reinterpret_cast<T*>(destination) = std::move(*reinterpret_cast<T*>(source));
        }
        else
        {
            call_at_index<N + 1, Targs...>::move(index, source, destination);
        }
    }

    static void moveConstructor(const uint64_t index, byte_t* source, byte_t* destination)
    {
        if (N == index)
        {
            new (destination) T(std::move(*reinterpret_cast<T*>(source)));
        }
        else
        {
            call_at_index<N + 1, Targs...>::moveConstructor(index, source, destination);
        }
    }

    static void copy(const uint64_t index, byte_t* source, byte_t* destination)
    {
        if (N == index)
        {
            *reinterpret_cast<T*>(destination) = *reinterpret_cast<T*>(source);
        }
        else
        {
            call_at_index<N + 1, Targs...>::copy(index, source, destination);
        }
    }

    static void copyConstructor(const uint64_t index, byte_t* source, byte_t* destination)
    {
        if (N == index)
        {
            new (destination) T(*reinterpret_cast<T*>(source));
        }
        else
        {
            call_at_index<N + 1, Targs...>::copyConstructor(index, source, destination);
        }
    }
};

template <uint64_t N, typename T>
struct call_at_index<N, T>
{
    static void destructor(const uint64_t index, byte_t* ptr)
    {
        if (N == index)
        {
            reinterpret_cast<T*>(ptr)->~T();
        }
        else
        {
            assert(false && "Could not call destructor for variant element");
        }
    }

    static void move(const uint64_t index, byte_t* source, byte_t* destination)
    {
        if (N == index)
        {
            *reinterpret_cast<T*>(destination) = std::move(*reinterpret_cast<T*>(source));
        }
        else
        {
            assert(false && "Could not call move assignment for variant element");
        }
    }

    static void moveConstructor(const uint64_t index, byte_t* source, byte_t* destination)
    {
        if (N == index)
        {
            new (destination) T(std::move(*reinterpret_cast<T*>(source)));
        }
        else
        {
            assert(false && "Could not call move constructor for variant element");
        }
    }

    static void copy(const uint64_t index, byte_t* source, byte_t* destination)
    {
        if (N == index)
        {
            *reinterpret_cast<T*>(destination) = *reinterpret_cast<T*>(source);
        }
        else
        {
            assert(false && "Could not call copy assignment for variant element");
        }
    }

    static void copyConstructor(const uint64_t index, byte_t* source, byte_t* destination)
    {
        if (N == index)
        {
            new (destination) T(*reinterpret_cast<T*>(source));
        }
        else
        {
            assert(false && "Could not call copy constructor for variant element");
        }
    }
};

} // namespace internal
} // namespace cxx
} // namespace iox

