// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_hoofs/cxx/requires.hpp"
#include "iceoryx_hoofs/internal/cxx/storable_function.hpp"

namespace iox
{
namespace cxx
{
template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename Functor, typename>
inline storable_function<Capacity, signature<ReturnType, Args...>>::storable_function(const Functor& functor) noexcept
{
    storeFunctor(functor);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline storable_function<Capacity, signature<ReturnType, Args...>>::storable_function(
    ReturnType (*function)(Args...)) noexcept
    : /// @NOLINTJUSTIFICATION we use type erasure in combination with compile time template arguments to restore
      ///                      the correct type whenever the callable is used
      /// @NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    m_callable(reinterpret_cast<void*>(function))
    , m_invoker(invokeFreeFunction)
{
    cxx::Expects(function);

    m_operations.copyFunction = copyFreeFunction;
    m_operations.moveFunction = moveFreeFunction;
    // destroy is not needed for free functions
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename T, typename>
inline storable_function<Capacity, signature<ReturnType, Args...>>::storable_function(
    T& object, ReturnType (T::*method)(Args...)) noexcept
{
    auto p = &object;
    auto functor = [p, method](Args... args) -> ReturnType { return (*p.*method)(std::forward<Args>(args)...); };
    storeFunctor(functor);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename T, typename>
inline storable_function<Capacity, signature<ReturnType, Args...>>::storable_function(const T& object,
                                                                                      ReturnType (T::*method)(Args...)
                                                                                          const) noexcept
{
    auto p = &object;
    auto functor = [p, method](Args... args) -> ReturnType { return (*p.*method)(std::forward<Args>(args)...); };
    storeFunctor(functor);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline storable_function<Capacity, signature<ReturnType, Args...>>::storable_function(
    const storable_function& other) noexcept
    : m_operations(other.m_operations)
    , m_invoker(other.m_invoker)
{
    m_operations.copy(other, *this);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline storable_function<Capacity, signature<ReturnType, Args...>>::storable_function(
    storable_function&& other) noexcept
    : m_operations(other.m_operations)
    , m_invoker(other.m_invoker)
{
    m_operations.move(other, *this);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline storable_function<Capacity, signature<ReturnType, Args...>>&
storable_function<Capacity, signature<ReturnType, Args...>>::operator=(const storable_function& rhs) noexcept
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

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline storable_function<Capacity, signature<ReturnType, Args...>>&
storable_function<Capacity, signature<ReturnType, Args...>>::operator=(storable_function&& rhs) noexcept
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

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline storable_function<Capacity, signature<ReturnType, Args...>>::~storable_function() noexcept
{
    m_operations.destroy(*this);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline ReturnType storable_function<Capacity, signature<ReturnType, Args...>>::operator()(Args... args) const noexcept
{
    return m_invoker(m_callable, std::forward<Args>(args)...);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline void storable_function<Capacity, signature<ReturnType, Args...>>::swap(storable_function& f) noexcept
{
    storable_function tmp = std::move(f);
    f = std::move(*this);
    *this = std::move(tmp);
}

template <uint64_t Capacity, typename T>
inline void swap(storable_function<Capacity, T>& f, storable_function<Capacity, T>& g) noexcept
{
    f.swap(g);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename Functor, typename>
inline void storable_function<Capacity, signature<ReturnType, Args...>>::storeFunctor(const Functor& functor) noexcept
{
    using StoredType = typename std::remove_reference<Functor>::type;
    auto ptr = m_storage.template allocate<StoredType>();
    cxx::Expects(ptr != nullptr);

    /// @NOLINTJUSTIFICATION ownership is encapsulated in storable_function and will be released in destroy
    /// @NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    ptr = new (ptr) StoredType(functor);

    // erase the functor type and store as reference to the call in storage
    m_callable = ptr;
    m_invoker = invoke<StoredType>;
    m_operations.copyFunction = copy<StoredType>;
    m_operations.moveFunction = move<StoredType>;
    m_operations.destroyFunction = destroy<StoredType>;
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename CallableType>
inline void storable_function<Capacity, signature<ReturnType, Args...>>::copy(const storable_function& src,
                                                                              storable_function& dest) noexcept
{
    auto ptr = dest.m_storage.template allocate<CallableType>();
    cxx::Expects(ptr != nullptr);

    auto obj = static_cast<CallableType*>(src.m_callable);
    /// @NOLINTJUSTIFICATION ownership is encapsulated in storable_function and will be released in destroy
    /// @NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    ptr = new (ptr) CallableType(*obj);
    dest.m_callable = ptr;
    dest.m_invoker = src.m_invoker;
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename CallableType>
inline void storable_function<Capacity, signature<ReturnType, Args...>>::move(storable_function& src,
                                                                              storable_function& dest) noexcept
{
    auto ptr = dest.m_storage.template allocate<CallableType>();
    cxx::Expects(ptr != nullptr);

    auto obj = static_cast<CallableType*>(src.m_callable);
    /// @NOLINTJUSTIFICATION ownership is encapsulated in storable_function and will be released in destroy
    /// @NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    ptr = new (ptr) CallableType(std::move(*obj));
    dest.m_callable = ptr;
    dest.m_invoker = src.m_invoker;
    src.m_operations.destroy(src);
    src.m_callable = nullptr;
    src.m_invoker = nullptr;
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename CallableType>
inline void storable_function<Capacity, signature<ReturnType, Args...>>::destroy(storable_function& f) noexcept
{
    if (f.m_callable)
    {
        auto ptr = static_cast<CallableType*>(f.m_callable);
        ptr->~CallableType();
        f.m_storage.deallocate();
    }
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline void
storable_function<Capacity, signature<ReturnType, Args...>>::copyFreeFunction(const storable_function& src,
                                                                              storable_function& dest) noexcept
{
    dest.m_invoker = src.m_invoker;
    dest.m_callable = src.m_callable;
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline void
storable_function<Capacity, signature<ReturnType, Args...>>::moveFreeFunction(storable_function& src,
                                                                              storable_function& dest) noexcept
{
    dest.m_invoker = src.m_invoker;
    dest.m_callable = src.m_callable;
    src.m_invoker = nullptr;
    src.m_callable = nullptr;
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename CallableType>
inline ReturnType storable_function<Capacity, signature<ReturnType, Args...>>::invoke(void* callable, Args&&... args)
{
    return (*static_cast<CallableType*>(callable))(std::forward<Args>(args)...);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline ReturnType storable_function<Capacity, signature<ReturnType, Args...>>::invokeFreeFunction(void* callable,
                                                                                                  Args&&... args)
{
    /// @NOLINTJUSTIFICATION we use type erasure in combination with compile time template arguments to restore
    ///                      the correct type whenever the callable is used
    /// @NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return (reinterpret_cast<ReturnType (*)(Args...)>(callable))(std::forward<Args>(args)...);
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename T>
inline constexpr uint64_t storable_function<Capacity, signature<ReturnType, Args...>>::required_storage_size() noexcept
{
    return StorageType::template allocation_size<T>();
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
template <typename T>
inline constexpr bool storable_function<Capacity, signature<ReturnType, Args...>>::is_storable() noexcept
{
    return (required_storage_size<T>() <= StorageType::capacity()) && is_invocable_r<ReturnType, T, Args...>::value;
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline void
storable_function<Capacity, signature<ReturnType, Args...>>::operations::copy(const storable_function& src,
                                                                              storable_function& dest) noexcept
{
    if (copyFunction)
    {
        copyFunction(src, dest);
    }
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline void
storable_function<Capacity, signature<ReturnType, Args...>>::operations::move(storable_function& src,
                                                                              storable_function& dest) noexcept
{
    if (moveFunction)
    {
        moveFunction(src, dest);
    }
}

template <uint64_t Capacity, typename ReturnType, typename... Args>
inline void
storable_function<Capacity, signature<ReturnType, Args...>>::operations::destroy(storable_function& f) noexcept
{
    if (destroyFunction)
    {
        destroyFunction(f);
    }
}

} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_STORABLE_FUNCTION_INL
