// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_UTILS_STORABLE_FUNCTION_HPP
#define IOX_UTILS_STORABLE_FUNCTION_HPP

#include "iceoryx_utils/cxx/function_ref.hpp"
#include "iceoryx_utils/cxx/type_traits.hpp"
#include "iceoryx_utils/internal/cxx/static_storage.hpp"

#include <iostream>
#include <type_traits>
#include <utility>

namespace iox
{
namespace cxx
{
template <typename ReturnType, typename... Args>
using signature = ReturnType(Args...);

template <typename StorageType, typename T>
class storable_function;

template <typename StorageType, typename ReturnType, typename... Args>
class storable_function<StorageType, signature<ReturnType, Args...>>
{
  private:
    // note that this vtable or a similar approach with virtual is needed to ensure we perform the correct
    // operation with the underlying (erased) type
    // this means storable_function cannot be used where pointers become invalid, e.g. across process boundaries
    struct vtable
    {
        // exposing those is intentional, it is an internal structure
        void (*copyFunction)(const storable_function& src, storable_function& dest){nullptr};
        void (*moveFunction)(storable_function& src, storable_function& dest){nullptr};
        void (*destroyFunction)(storable_function& f){nullptr};

        vtable() = default;
        vtable(const vtable& other) = default;
        vtable& operator=(const vtable& other) = default;
        vtable(vtable&& other) = default;
        vtable& operator=(vtable&& other) = default;

        void copy(const storable_function& src, storable_function& dest)
        {
            if (copyFunction)
            {
                copyFunction(src, dest);
            }
        }

        void move(storable_function& src, storable_function& dest)
        {
            if (moveFunction)
            {
                moveFunction(src, dest);
            }
        }

        void destroy(storable_function& f)
        {
            if (destroyFunction)
            {
                destroyFunction(f);
            }
        }
    };

  public:
    using signature_t = signature<ReturnType, Args...>;

    storable_function() = default;

    /// @brief construct from functor (including lambda)
    template <typename Functor,
              typename = typename std::enable_if<std::is_class<Functor>::value
                                                     && is_invocable_r<ReturnType, Functor, Args...>::value,
                                                 void>::type>
    storable_function(const Functor& functor) noexcept;

    /// @brief construct from function pointer (including static functions)
    storable_function(ReturnType (*function)(Args...)) noexcept;

    /// @brief construct from object reference and member function
    /// only a pointer to the object is stored for the call
    template <typename T, typename = typename std::enable_if<std::is_class<T>::value, void>::type>
    storable_function(T& object, ReturnType (T::*method)(Args...)) noexcept;

    /// @brief construct from object reference and const member function
    /// only a pointer to the object is stored for the call
    template <typename T, typename = typename std::enable_if<std::is_class<T>::value, void>::type>
    storable_function(const T& object, ReturnType (T::*method)(Args...) const) noexcept;

    storable_function(const storable_function& other) noexcept;

    storable_function(storable_function&& other) noexcept;

    storable_function& operator=(const storable_function& rhs) noexcept;

    storable_function& operator=(storable_function&& rhs) noexcept;

    ~storable_function() noexcept;

    /// @brief invoke the stored function
    ReturnType operator()(Args... args);

    /// @brief indicates whether a function was stored
    operator bool() noexcept;

    /// @brief swap this with another function
    void swap(storable_function& f) noexcept;

    /// @brief swap two functions
    static void swap(storable_function& f, storable_function& g) noexcept;

    /// @brief size in bytes required to store a type T in a storable_function
    /// @note this is not exact due to alignment, it may work with a smaller size but
    ///       is not guaranteed
    template <typename T>
    static constexpr uint64_t storage_bytes_required() noexcept;

    /// @brief checks whether T is storable
    /// @return true if it can be stored, false if it is not guaranteed that it can be stored
    /// @note it might be storable for some alignments of T even if it returns false,
    ///       in this case it is advised to increase the size of storage via the StorageType
    template <typename T>
    static constexpr bool is_storable() noexcept;

  private:
    vtable m_vtable;
    StorageType m_storage;
    void* m_storedObj{nullptr};
    function_ref<signature<ReturnType, Args...>> m_function;

    template <typename Functor,
              typename = typename std::enable_if<std::is_class<Functor>::value
                                                     && is_invocable_r<ReturnType, Functor, Args...>::value,
                                                 void>::type>
    void storeFunctor(const Functor& functor) noexcept;

    // we need these templates to preserve the actual type T for the underlying copy/move etc. call
    template <typename T>
    static void copy(const storable_function& src, storable_function& dest) noexcept;

    template <typename T>
    static void move(storable_function& src, storable_function& dest) noexcept;

    template <typename T>
    static void destroy(storable_function& f) noexcept;

    static void copyFreeFunction(const storable_function& src, storable_function& dest) noexcept;

    static void moveFreeFunction(storable_function& src, storable_function& dest) noexcept;
};

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/storable_function.inl"

#endif // IOX_UTILS_STORABLE_FUNCTION_HPP
