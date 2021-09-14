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

#ifndef IOX_HOOFS_CXX_METHOD_CALLBACK_INL
#define IOX_HOOFS_CXX_METHOD_CALLBACK_INL

#include "iceoryx_hoofs/cxx/method_callback.hpp"
namespace iox
{
namespace cxx
{
namespace internal
{
template <typename ReturnValue>
struct ReturnSuccess
{
    template <typename Callback, typename... Args>
    static success<ReturnValue> call(Callback& callback, Args&&... args) noexcept
    {
        return success<ReturnValue>(callback(std::forward<Args>(args)...));
    }
};

template <>
struct ReturnSuccess<void>
{
    template <typename Callback, typename... Args>
    static success<void> call(Callback& callback, Args&&... args) noexcept
    {
        callback(std::forward<Args>(args)...);
        return success<void>();
    }
};

template <typename ReturnValue, typename ClassType, typename... Args>
ReturnValue constMethodCallbackCaller(const void* objectRef,
                                      ReturnValue (GenericClass::*methodPtr)(Args...) const,
                                      Args&&... args) noexcept
{
    return ((*reinterpret_cast<const ClassType*>(objectRef))
            .*reinterpret_cast<ReturnValue (ClassType::*)(Args...) const>(methodPtr))(std::forward<Args>(args)...);
}


template <typename ReturnValue, typename ClassType, typename... Args>
ReturnValue
methodCallbackCaller(void* objectRef, ReturnValue (GenericClass::*methodPtr)(Args...), Args&&... args) noexcept
{
    return ((*reinterpret_cast<ClassType*>(objectRef))
            .*reinterpret_cast<ReturnValue (ClassType::*)(Args...)>(methodPtr))(std::forward<Args>(args)...);
}
} // namespace internal


template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline ConstMethodCallback<ReturnValue, Args...>::ConstMethodCallback(const ClassType& objectRef,
                                                                      ReturnValue (ClassType::*methodPtr)(Args...)
                                                                          const) noexcept
    : m_objectPtr(&objectRef)
    , m_methodPtr(reinterpret_cast<ConstMethodPointer<internal::GenericClass>>(methodPtr))
    , m_callback(internal::constMethodCallbackCaller<ReturnValue, ClassType, Args...>)
{
    cxx::Expects(m_methodPtr != nullptr);
}

template <typename ReturnValue, typename... Args>
inline ConstMethodCallback<ReturnValue, Args...>::ConstMethodCallback(ConstMethodCallback&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <typename ReturnValue, typename... Args>
inline ConstMethodCallback<ReturnValue, Args...>&
ConstMethodCallback<ReturnValue, Args...>::operator=(ConstMethodCallback&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_objectPtr = std::move(rhs.m_objectPtr);
        m_methodPtr = std::move(rhs.m_methodPtr);
        m_callback = std::move(rhs.m_callback);

        rhs.m_objectPtr = nullptr;
    }

    return *this;
}

template <typename ReturnValue, typename... Args>
template <typename... MethodArguments>
inline expected<ReturnValue, MethodCallbackError>
ConstMethodCallback<ReturnValue, Args...>::operator()(MethodArguments&&... args) const noexcept
{
    if (!isValid())
    {
        return error<MethodCallbackError>(MethodCallbackError::UNINITIALIZED_CALLBACK);
    }

    return internal::ReturnSuccess<ReturnValue>::call(
        m_callback, m_objectPtr, m_methodPtr, std::forward<MethodArguments>(args)...);
}

template <typename ReturnValue, typename... Args>
inline bool ConstMethodCallback<ReturnValue, Args...>::operator==(const ConstMethodCallback& rhs) const noexcept
{
    return (m_objectPtr == rhs.m_objectPtr && m_methodPtr == rhs.m_methodPtr);
}

template <typename ReturnValue, typename... Args>
inline bool ConstMethodCallback<ReturnValue, Args...>::operator!=(const ConstMethodCallback& rhs) const noexcept
{
    return !(this->operator==(rhs));
}

template <typename ReturnValue, typename... Args>
inline bool ConstMethodCallback<ReturnValue, Args...>::isValid() const noexcept
{
    return m_objectPtr != nullptr && m_methodPtr != nullptr;
}

template <typename ReturnValue, typename... Args>
inline ConstMethodCallback<ReturnValue, Args...>::operator bool() const noexcept
{
    return isValid();
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline void ConstMethodCallback<ReturnValue, Args...>::setCallback(const ClassType& objectRef,
                                                                   ConstMethodPointer<ClassType> methodPtr) noexcept
{
    cxx::Expects(methodPtr != nullptr);

    m_objectPtr = &objectRef;
    m_methodPtr = reinterpret_cast<ConstMethodPointer<internal::GenericClass>>(methodPtr);
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline const ClassType* ConstMethodCallback<ReturnValue, Args...>::getObjectPointer() const noexcept
{
    return reinterpret_cast<const ClassType*>(m_objectPtr);
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline auto ConstMethodCallback<ReturnValue, Args...>::getMethodPointer() const noexcept
    -> ConstMethodPointer<ClassType>
{
    return reinterpret_cast<ConstMethodPointer<ClassType>>(m_methodPtr);
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline MethodCallback<ReturnValue, Args...>::MethodCallback(ClassType& objectRef,
                                                            MethodPointer<ClassType> methodPtr) noexcept
    : m_objectPtr(&objectRef)
    , m_methodPtr(reinterpret_cast<MethodPointer<internal::GenericClass>>(methodPtr))
    , m_callback(internal::methodCallbackCaller<ReturnValue, ClassType, Args...>)
{
    cxx::Expects(m_methodPtr != nullptr);
}

template <typename ReturnValue, typename... Args>
inline MethodCallback<ReturnValue, Args...>::MethodCallback(MethodCallback&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <typename ReturnValue, typename... Args>
inline MethodCallback<ReturnValue, Args...>&
MethodCallback<ReturnValue, Args...>::operator=(MethodCallback&& rhs) noexcept
{
    if (this != &rhs)
    {
        m_objectPtr = std::move(rhs.m_objectPtr);
        m_methodPtr = std::move(rhs.m_methodPtr);
        m_callback = std::move(rhs.m_callback);

        rhs.m_objectPtr = nullptr;
    }

    return *this;
}

template <typename ReturnValue, typename... Args>
template <typename... MethodArguments>
inline expected<ReturnValue, MethodCallbackError>
MethodCallback<ReturnValue, Args...>::operator()(MethodArguments&&... args) noexcept
{
    if (!isValid())
    {
        return error<MethodCallbackError>(MethodCallbackError::UNINITIALIZED_CALLBACK);
    }

    return internal::ReturnSuccess<ReturnValue>::call(
        m_callback, m_objectPtr, m_methodPtr, std::forward<MethodArguments>(args)...);
}

template <typename ReturnValue, typename... Args>
inline bool MethodCallback<ReturnValue, Args...>::operator==(const MethodCallback& rhs) const noexcept
{
    return (m_objectPtr == rhs.m_objectPtr && m_methodPtr == rhs.m_methodPtr);
}

template <typename ReturnValue, typename... Args>
inline bool MethodCallback<ReturnValue, Args...>::operator!=(const MethodCallback& rhs) const noexcept
{
    return !(this->operator==(rhs));
}

template <typename ReturnValue, typename... Args>
inline bool MethodCallback<ReturnValue, Args...>::isValid() const noexcept
{
    return m_objectPtr != nullptr && m_methodPtr != nullptr;
}

template <typename ReturnValue, typename... Args>
inline MethodCallback<ReturnValue, Args...>::operator bool() const noexcept
{
    return isValid();
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline void MethodCallback<ReturnValue, Args...>::setCallback(ClassType& objectRef,
                                                              MethodPointer<ClassType> methodPtr) noexcept
{
    cxx::Expects(methodPtr != nullptr);

    m_objectPtr = &objectRef;
    m_methodPtr = reinterpret_cast<MethodPointer<internal::GenericClass>>(methodPtr);
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline ClassType* MethodCallback<ReturnValue, Args...>::getObjectPointer() const noexcept
{
    return reinterpret_cast<ClassType*>(m_objectPtr);
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline auto MethodCallback<ReturnValue, Args...>::getMethodPointer() const noexcept -> MethodPointer<ClassType>
{
    return reinterpret_cast<MethodPointer<ClassType>>(m_methodPtr);
}

} // namespace cxx
} // namespace iox
#endif
