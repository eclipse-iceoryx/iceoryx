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

#ifndef IOX_UTILS_STORABLE_FUNCTION_INL
#define IOX_UTILS_STORABLE_FUNCTION_INL

#include "iceoryx_utils/internal/cxx/storable_function.hpp"

namespace iox
{
namespace cxx
{
template <typename S, typename ReturnType, typename... Args>
template <typename Functor, typename>
storable_function<S, signature<ReturnType, Args...>>::storable_function(const Functor& functor) noexcept
{
    storeFunctor(functor);
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>::storable_function(ReturnType (*function)(Args...)) noexcept
{
    m_function = function;
    m_storedObj = nullptr;
    m_vtable.copyFunction = copyFreeFunction;
    m_vtable.moveFunction = moveFreeFunction;
    // destroy is not needed for free functions
}

template <typename S, typename ReturnType, typename... Args>
template <typename T, typename>
storable_function<S, signature<ReturnType, Args...>>::storable_function(T& object,
                                                                        ReturnType (T::*method)(Args...)) noexcept
{
    auto p = &object;
    auto functor = [p, method](Args... args) -> ReturnType { return (*p.*method)(std::forward<Args>(args)...); };
    storeFunctor(functor);
}

template <typename S, typename ReturnType, typename... Args>
template <typename T, typename>
storable_function<S, signature<ReturnType, Args...>>::storable_function(const T& object,
                                                                        ReturnType (T::*method)(Args...) const) noexcept
{
    auto p = &object;
    auto functor = [p, method](Args... args) -> ReturnType { return (*p.*method)(std::forward<Args>(args)...); };
    storeFunctor(functor);
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>::storable_function(const storable_function& other) noexcept
    : m_vtable(other.m_vtable)
{
    m_vtable.copy(other, *this);
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>::storable_function(storable_function&& other) noexcept
    : m_vtable(other.m_vtable)
{
    m_vtable.move(other, *this);
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>&
storable_function<S, signature<ReturnType, Args...>>::operator=(const storable_function& rhs) noexcept
{
    if (&rhs != this)
    {
        // note: vtable is needed for destroy, then changed to src vtable
        m_vtable.destroy(*this);
        m_function = nullptr; // only needed when the src has no object
        m_vtable = rhs.m_vtable;
        m_vtable.copy(rhs, *this);
    }

    return *this;
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>&
storable_function<S, signature<ReturnType, Args...>>::operator=(storable_function&& rhs) noexcept
{
    if (&rhs != this)
    {
        // note: dest (this) vtable is needed for destroy, then changed to source (rhs) vtable
        m_vtable.destroy(*this);
        m_function = nullptr; // only needed when the source has no object
        m_vtable = rhs.m_vtable;
        m_vtable.move(rhs, *this);
    }

    return *this;
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>::~storable_function() noexcept
{
    m_vtable.destroy(*this);
}


template <typename S, typename ReturnType, typename... Args>
ReturnType storable_function<S, signature<ReturnType, Args...>>::operator()(Args... args)
{
    auto r = m_function(std::forward<Args>(args)...);
    return r;
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>::operator bool() noexcept
{
    return m_function.operator bool();
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::swap(storable_function& f) noexcept
{
    storable_function tmp = std::move(f);
    f = std::move(*this);
    *this = std::move(tmp);
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::swap(storable_function& f, storable_function& g) noexcept
{
    storable_function tmp = std::move(f);
    f = std::move(g);
    g = std::move(tmp);
}

template <typename S, typename ReturnType, typename... Args>
template <typename Functor, typename>
void storable_function<S, signature<ReturnType, Args...>>::storeFunctor(const Functor& functor) noexcept
{
    ///@todo we may need something more to get the correct type in general...
    using StoredType = typename std::remove_reference<Functor>::type;
    auto p = m_storage.template allocate<StoredType>();

    if (p)
    {
        // functor will fit, copy it
        p = new (p) StoredType(functor);

        // erase the functor type and store as reference to the call in storage
        m_function = *p;
        m_storedObj = p;
        m_vtable.copyFunction = copy<StoredType>;
        m_vtable.moveFunction = move<StoredType>;
        m_vtable.destroyFunction = destroy<StoredType>;
    }

    // else we detect the problem at compile time or store nothing when memory is exhausted
    // note that we have no other choice, it is used in the ctor and we cannot throw
    // the object will be valid but not callable (operator bool returns false)

    // if used with static_storage we always detect this at compile time
}

template <typename S, typename ReturnType, typename... Args>
template <typename T>
void storable_function<S, signature<ReturnType, Args...>>::copy(const storable_function& src,
                                                                storable_function& dest) noexcept
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

template <typename S, typename ReturnType, typename... Args>
template <typename T>
void storable_function<S, signature<ReturnType, Args...>>::move(storable_function& src,
                                                                storable_function& dest) noexcept
{
    if (!src.m_storedObj)
    {
        dest.m_storedObj = nullptr;
        dest.m_function = src.m_function;
        src.m_function = nullptr;
        return;
    }

    auto p = dest.m_storage.template allocate<T>();
    if (p)
    {
        auto obj = reinterpret_cast<T*>(src.m_storedObj);
        p = new (p) T(std::move(*obj));
        dest.m_function = *p;
        dest.m_storedObj = p;
        src.m_vtable.destroy(src);
        src.m_function = nullptr;
        src.m_storedObj = nullptr;
    }
}

template <typename S, typename ReturnType, typename... Args>
template <typename T>
void storable_function<S, signature<ReturnType, Args...>>::destroy(storable_function& f) noexcept
{
    if (f.m_storedObj)
    {
        auto p = static_cast<T*>(f.m_storedObj);
        p->~T();
        f.m_storage.deallocate();
    }
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::copyFreeFunction(const storable_function& src,
                                                                            storable_function& dest) noexcept
{
    dest.m_function = src.m_function;
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::moveFreeFunction(storable_function& src,
                                                                            storable_function& dest) noexcept
{
    dest.m_function = src.m_function;
    src.m_function = nullptr;
}

template <typename S, typename ReturnType, typename... Args>
template <typename T>
constexpr uint64_t storable_function<S, signature<ReturnType, Args...>>::storage_bytes_required() noexcept
{
    return sizeof(T) + alignof(T);
}

template <typename S, typename ReturnType, typename... Args>
template <typename T>
constexpr bool storable_function<S, signature<ReturnType, Args...>>::is_storable() noexcept
{
    return (storage_bytes_required<T>() <= S::capacity()) && is_invocable_r<ReturnType, T, Args...>::value;
}

} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_STORABLE_FUNCTION_INL