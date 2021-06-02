// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_STORABLE_FUNCTION_INL
#define IOX_HOOFS_STORABLE_FUNCTION_INL

#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/internal/cxx/storable_function.hpp"

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
    if (function)
    {
        m_invoker = invokeFreeFunction;
        m_callable = reinterpret_cast<void*>(function);
        m_operations.copyFunction = copyFreeFunction;
        m_operations.moveFunction = moveFreeFunction;
        // destroy is not needed for free functions
    }
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
    : m_operations(other.m_operations)
    , m_invoker(other.m_invoker)
{
    m_operations.copy(other, *this);
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>::storable_function(storable_function&& other) noexcept
    : m_operations(other.m_operations)
    , m_invoker(other.m_invoker)
{
    m_operations.move(other, *this);
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>&
storable_function<S, signature<ReturnType, Args...>>::operator=(const storable_function& rhs) noexcept
{
    if (&rhs != this)
    {
        // this operations is needed for destroy, then changed to source (rhs) operations
        m_operations.destroy(*this);
        m_operations = rhs.m_operations;
        m_invoker = rhs.m_invoker;
        m_operations.copy(rhs, *this);
    }

    return *this;
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>&
storable_function<S, signature<ReturnType, Args...>>::operator=(storable_function&& rhs) noexcept
{
    if (&rhs != this)
    {
        // this operations is needed for destroy, then changed to source (rhs) operations
        m_operations.destroy(*this);
        m_operations = rhs.m_operations;
        m_invoker = rhs.m_invoker;
        m_operations.move(rhs, *this);
    }

    return *this;
}

template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>::~storable_function() noexcept
{
    m_operations.destroy(*this);
}

template <typename S, typename ReturnType, typename... Args>
ReturnType storable_function<S, signature<ReturnType, Args...>>::operator()(Args... args)
{
    cxx::Expects(!empty());
    return m_invoker(m_callable, std::forward<Args>(args)...);
}


template <typename S, typename ReturnType, typename... Args>
storable_function<S, signature<ReturnType, Args...>>::operator bool() noexcept
{
    return !empty();
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::swap(storable_function& f) noexcept
{
    storable_function tmp = std::move(f);
    f = std::move(*this);
    *this = std::move(tmp);
}

template <typename S, typename T>
void swap(storable_function<S, T>& f, storable_function<S, T>& g) noexcept
{
    f.swap(g);
}

template <typename S, typename ReturnType, typename... Args>
bool storable_function<S, signature<ReturnType, Args...>>::empty() const noexcept
{
    return m_invoker == nullptr;
}

template <typename S, typename ReturnType, typename... Args>
template <typename Functor, typename>
void storable_function<S, signature<ReturnType, Args...>>::storeFunctor(const Functor& functor) noexcept
{
    using StoredType = typename std::remove_reference<Functor>::type;
    auto ptr = m_storage.template allocate<StoredType>();

    if (ptr)
    {
        // functor will fit, copy it
        ptr = new (ptr) StoredType(functor);

        // erase the functor type and store as reference to the call in storage
        m_callable = ptr;
        m_invoker = invoke<StoredType>;
        m_operations.copyFunction = copy<StoredType>;
        m_operations.moveFunction = move<StoredType>;
        m_operations.destroyFunction = destroy<StoredType>;
    }
    else
    {
        std::cerr << "storable_function: no memory to store functor\n";
        // this cannot happen in the static_storage case
    }

    cxx::Ensures(!empty());
}

template <typename S, typename ReturnType, typename... Args>
template <typename CallableType>
void storable_function<S, signature<ReturnType, Args...>>::copy(const storable_function& src,
                                                                storable_function& dest) noexcept
{
    if (src.empty())
    {
        // nothing to do, destroy and setting m_invoker to nullptr are performed before the type specific copy
        // operation
        return;
    }

    auto ptr = dest.m_storage.template allocate<CallableType>();

    if (ptr)
    {
        auto obj = static_cast<CallableType*>(src.m_callable);
        ptr = new (ptr) CallableType(*obj);
        dest.m_callable = ptr;
        dest.m_invoker = src.m_invoker;
    }
    else
    {
        std::cerr << "storable_function: no memory to store copy at destination\n";
        // no memory avilable in source, assignment could not be performed
        // (this cannot happen in the static_storage case)
    }

    cxx::Ensures(!dest.empty());
}

template <typename S, typename ReturnType, typename... Args>
template <typename CallableType>
void storable_function<S, signature<ReturnType, Args...>>::move(storable_function& src,
                                                                storable_function& dest) noexcept
{
    if (src.empty())
    {
        // nothing to do, destroy and setting m_invoker to nullptr are performed before the type specific move
        // operation
        return;
    }

    auto ptr = dest.m_storage.template allocate<CallableType>();
    if (ptr)
    {
        auto obj = static_cast<CallableType*>(src.m_callable);
        ptr = new (ptr) CallableType(std::move(*obj));
        dest.m_callable = ptr;
        dest.m_invoker = src.m_invoker;
        src.m_operations.destroy(src);
        src.m_callable = nullptr;
        src.m_invoker = nullptr;
    }
    else
    {
        // no memory in source, assignment could not be performed
        // (this cannot happen in the static_storage case)
        std::cerr << "storable_function: no memory to store moved object at destination\n";
    }

    cxx::Ensures(!dest.empty());
}

template <typename S, typename ReturnType, typename... Args>
template <typename CallableType>
void storable_function<S, signature<ReturnType, Args...>>::destroy(storable_function& f) noexcept
{
    if (f.m_callable)
    {
        auto ptr = static_cast<CallableType*>(f.m_callable);
        ptr->~CallableType();
        f.m_storage.deallocate();
    }
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::copyFreeFunction(const storable_function& src,
                                                                            storable_function& dest) noexcept
{
    dest.m_invoker = src.m_invoker;
    dest.m_callable = src.m_callable;
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::moveFreeFunction(storable_function& src,
                                                                            storable_function& dest) noexcept
{
    dest.m_invoker = src.m_invoker;
    dest.m_callable = src.m_callable;
    src.m_invoker = nullptr;
    src.m_callable = nullptr;
}

template <typename S, typename ReturnType, typename... Args>
template <typename CallableType>
ReturnType storable_function<S, signature<ReturnType, Args...>>::invoke(void* callable, Args&&... args)
{
    return (*static_cast<CallableType*>(callable))(std::forward<Args>(args)...);
}

template <typename S, typename ReturnType, typename... Args>
ReturnType storable_function<S, signature<ReturnType, Args...>>::invokeFreeFunction(void* callable, Args&&... args)
{
    return (reinterpret_cast<ReturnType (*)(Args...)>(callable))(std::forward<Args>(args)...);
}

template <typename S, typename ReturnType, typename... Args>
template <typename T>
constexpr uint64_t storable_function<S, signature<ReturnType, Args...>>::required_storage_size() noexcept
{
    return S::template allocation_size<T>();
}

template <typename S, typename ReturnType, typename... Args>
template <typename T>
constexpr bool storable_function<S, signature<ReturnType, Args...>>::is_storable() noexcept
{
    return (required_storage_size<T>() <= S::capacity()) && is_invocable_r<ReturnType, T, Args...>::value;
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::operations::copy(const storable_function& src,
                                                                            storable_function& dest) noexcept
{
    if (copyFunction)
    {
        copyFunction(src, dest);
    }
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::operations::move(storable_function& src,
                                                                            storable_function& dest) noexcept
{
    if (moveFunction)
    {
        moveFunction(src, dest);
    }
}

template <typename S, typename ReturnType, typename... Args>
void storable_function<S, signature<ReturnType, Args...>>::operations::destroy(storable_function& f) noexcept
{
    if (destroyFunction)
    {
        destroyFunction(f);
    }
}

} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_STORABLE_FUNCTION_INL