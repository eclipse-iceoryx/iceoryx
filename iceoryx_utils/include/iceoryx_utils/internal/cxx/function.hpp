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

#ifndef IOX_UTILS_FUNCTION_HPP
#define IOX_UTILS_FUNCTION_HPP


// replace later when the error in function_ref is found
#if 0
#include "iceoryx_utils/cxx/function_ref.hpp"
#else
#include "iceoryx_utils/internal/cxx/function_ref_alternative.hpp"
#endif

#include "iceoryx_utils/cxx/type_traits.hpp"
#include "iceoryx_utils/internal/cxx/storage.hpp"

#include <iostream>
#include <type_traits>
#include <utility>

namespace iox
{
namespace cxx
{
namespace detail
{
template <typename ReturnType, typename... Args>
using signature = ReturnType(Args...);

template <typename StorageType, typename T>
class storable_function;

template <typename StorageType, typename ReturnType, typename... Args>
class storable_function<StorageType, signature<ReturnType, Args...>>
{
  private:
    ///@todo: move to detail namespace, move to inl once complete

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

    // todo: need is_invocable_r (has_signature does not work) to let compilation fail for non-invocable types
    /// @brief construct from functor (including lambda)
    template <typename Functor,
              typename = typename std::enable_if<std::is_class<Functor>::value
                                                 /*&& is_invocable_r<ReturnType, Functor, Args...>::value*/,
                                                 void>::type>
    storable_function(const Functor& functor) noexcept
    {
        storeFunctor(functor);
    }

    /// @brief construct from function pointer (including static functions)
    storable_function(ReturnType (*function)(Args...)) noexcept
    {
        std::cout << "from fptr" << std::endl;
        // no need to store anything else (m_storage is not needed here)

        // major todo: storing this function pointer and calling it later will not work
        // with the iceoryx function_ref, why?
        // use an alternative and investiagte later
        m_function = function;
        m_storedObj = nullptr;
    }

    /// @brief construct from object reference and member function
    /// only a pointer to the object is stored for the call
    template <typename T, typename = typename std::enable_if<std::is_class<T>::value, void>::type>
    storable_function(T& object, ReturnType (T::*method)(Args...)) noexcept
    {
        auto p = &object;
        auto functor = [p, method](Args... args) -> ReturnType { return (*p.*method)(std::forward<Args>(args)...); };
        storeFunctor(functor);
    }

    /// @brief construct from object reference and const member function
    /// only a pointer to the object is stored for the call
    template <typename T, typename = typename std::enable_if<std::is_class<T>::value, void>::type>
    storable_function(const T& object, ReturnType (T::*method)(Args...) const) noexcept
    {
        auto p = &object;
        auto functor = [p, method](Args... args) -> ReturnType { return (*p.*method)(std::forward<Args>(args)...); };
        storeFunctor(functor);
    }

    /// @brief copy construct a function
    storable_function(const storable_function& other) noexcept
        : m_vtable(other.m_vtable)
    {
        m_vtable.copy(other, *this);
    }

    /// @brief move construct a function
    storable_function(storable_function&& other) noexcept
        : m_vtable(other.m_vtable)
    {
        m_vtable.move(other, *this);
    }

    /// @brief copy assign a function
    storable_function& operator=(const storable_function& rhs) noexcept
    {
        if (&rhs != this)
        {
            m_vtable = rhs.m_vtable;
            m_vtable.destroy(*this);
            m_vtable.copy(rhs, *this);
        }

        return *this;
    }

    /// @brief move assign a function
    storable_function& operator=(storable_function&& rhs) noexcept
    {
        if (&rhs != this)
        {
            m_vtable = rhs.m_vtable;
            m_vtable.destroy(*this);
            m_vtable.move(rhs, *this);
        }

        return *this;
    }

    /// @brief destroy the function
    ~storable_function() noexcept
    {
        m_vtable.destroy(*this);
    }

    /// @brief invoke the stored function
    // todo: think about constness, calling it may change the stored object
    ReturnType operator()(Args... args)
    {
        std::cout << "invoke" << std::endl;
        auto r = m_function(std::forward<Args>(args)...);
        std::cout << "invoked" << std::endl;
        return r;
    }

    /// @brief indicates whether a function was stored
    operator bool() noexcept
    {
        return m_function.operator bool();
    }

    /// @brief swap two functions
    void swap(storable_function& f, storable_function& g) noexcept
    {
        storable_function tmp = std::move(f);
        f = std::move(g);
        g = std::move(tmp);
    }

  private:
    /// @note  in general we cannot know the alignment of the type we want to store at construction time
    StorageType m_storage;
    void* m_storedObj{nullptr};
    function_ref<signature<ReturnType, Args...>> m_function;
    vtable m_vtable;

    template <typename Functor,
              typename = typename std::enable_if<std::is_class<Functor>::value
                                                 /*&& is_invocable_r<ReturnType, Functor, Args...>::value*/,
                                                 void>::type>
    void storeFunctor(const Functor& functor) noexcept
    {
        ///@todo we may need something more to get the correct type in general...
        using StoredType = typename std::remove_reference<Functor>::type;
        auto p = m_storage.template allocate<StoredType>();

        if (p)
        {
            // functor will fit, copy it
            new (p) StoredType(functor);

            // erase the functor type and store as reference to the call in storage
            m_function = *p;
            m_storedObj = p;
            m_vtable.copyFunction = copy<StoredType>;
            m_vtable.moveFunction = move<StoredType>;
            m_vtable.destroyFunction = destroy<StoredType>;
        }
        // else handle problem, but there should be none when we have the heap fallback
        // otherwise we need to detect the size problem at compile time
    }

    // need these templates to preserve the actual type T for the underlying copy/move etc. call
    template <typename T>
    static void copy(const storable_function& src, storable_function& dest) noexcept
    {
        if (!src.m_storedObj)
        {
            dest.m_storedObj = nullptr;
            dest.m_function = src.m_function;
            return;
        }

        auto p = dest.m_storage.template allocate<T>();

        if (p)
        {
            auto obj = reinterpret_cast<T*>(src.m_storedObj);
            p = new (p) T(*obj);
            dest.m_function = *p;
            dest.m_storedObj = p;
        }
    }

    template <typename T>
    static void move(storable_function& src, storable_function& dest) noexcept
    {
        if (!src.m_storedObj)
        {
            dest.m_storedObj = nullptr;
            dest.m_function = src.m_function;
            return;
        }

        auto p = dest.m_storage.template allocate<T>();

        if (p)
        {
            auto obj = reinterpret_cast<T*>(src.m_storedObj);
            p = new (p) T(std::move(*obj));
            dest.m_function = *p;
            dest.m_storedObj = p;
        }
    }

    template <typename T>
    static void destroy(storable_function& f) noexcept
    {
        if (f.m_storedObj)
        {
            auto p = static_cast<T*>(f.m_storedObj);
            p->~T();
            f.m_storage.deallocate();
            f.m_storedObj = nullptr;
        }
    }
};

} // namespace detail
// exposed to the user, to set the storage type (and reorder template arguments)
// the storage type must precede the (required) variadic arguments in the internal one
template <typename Signature, uint64_t Bytes = 128>
using function = detail::storable_function<static_storage<Bytes>, Signature>;

} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_FUNCTION_HPP
