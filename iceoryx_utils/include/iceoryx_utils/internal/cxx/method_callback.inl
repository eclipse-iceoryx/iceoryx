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

#ifndef IOX_UTILS_CXX_METHOD_CALLBACK_INL
#define IOX_UTILS_CXX_METHOD_CALLBACK_INL

#include "iceoryx_utils/cxx/method_callback.hpp"
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
ReturnValue
constMethodCallbackCaller(void* classPtr, ReturnValue (GenericClass::*methodPtr)(Args...) const, Args&&... args)
{
    return ((*reinterpret_cast<ClassType*>(classPtr))
            .*reinterpret_cast<ReturnValue (ClassType::*)(Args...) const>(methodPtr))(std::forward<Args>(args)...);
}


template <typename ReturnValue, typename ClassType, typename... Args>
ReturnValue methodCallbackCaller(void* classPtr, ReturnValue (GenericClass::*methodPtr)(Args...), Args&&... args)
{
    return ((*reinterpret_cast<ClassType*>(classPtr))
            .*reinterpret_cast<ReturnValue (ClassType::*)(Args...)>(methodPtr))(std::forward<Args>(args)...);
}
} // namespace internal


template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline ConstMethodCallback<ReturnValue, Args...>::ConstMethodCallback(ClassType* const classPtr,
                                                                      ReturnValue (ClassType::*methodPtr)(Args...)
                                                                          const) noexcept
    : m_classPtr(classPtr)
    , m_methodPtr(reinterpret_cast<ReturnValue (internal::GenericClass::*)(Args...) const>(methodPtr))
    , m_callback(internal::constMethodCallbackCaller<ReturnValue, ClassType, Args...>)
{
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
        m_classPtr = rhs.m_classPtr;
        m_methodPtr = rhs.m_methodPtr;
        m_callback = rhs.m_callback;

        rhs.m_classPtr = nullptr;
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
        return error<MethodCallbackError>(MethodCallbackError::UNABLE_TO_CALL_METHOD_ON_NULLPTR_CLASS_PTR);
    }

    return internal::ReturnSuccess<ReturnValue>::call(
        m_callback, m_classPtr, m_methodPtr, std::forward<MethodArguments>(args)...);
}

template <typename ReturnValue, typename... Args>
inline bool ConstMethodCallback<ReturnValue, Args...>::operator==(const ConstMethodCallback& rhs) const noexcept
{
    return (m_classPtr == rhs.m_classPtr && m_methodPtr == rhs.m_methodPtr);
}

template <typename ReturnValue, typename... Args>
inline bool ConstMethodCallback<ReturnValue, Args...>::operator!=(const ConstMethodCallback& rhs) const noexcept
{
    return !(this->operator==(rhs));
}

template <typename ReturnValue, typename... Args>
inline bool ConstMethodCallback<ReturnValue, Args...>::isValid() const noexcept
{
    return m_classPtr != nullptr;
}

template <typename ReturnValue, typename... Args>
inline ConstMethodCallback<ReturnValue, Args...>::operator bool() const noexcept
{
    return isValid();
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline void ConstMethodCallback<ReturnValue, Args...>::setObjectPointer(ClassType* const classPtr) noexcept
{
    m_classPtr = classPtr;
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline ClassType* ConstMethodCallback<ReturnValue, Args...>::getClassPointer() const noexcept
{
    return reinterpret_cast<ClassType*>(m_classPtr);
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline MethodCallback<ReturnValue, Args...>::MethodCallback(ClassType* const classPtr,
                                                            ReturnValue (ClassType::*methodPtr)(Args...)) noexcept
    : m_classPtr(classPtr)
    , m_methodPtr(reinterpret_cast<ReturnValue (internal::GenericClass::*)(Args...)>(methodPtr))
    , m_callback(internal::methodCallbackCaller<ReturnValue, ClassType, Args...>)
{
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
        m_classPtr = rhs.m_classPtr;
        m_methodPtr = rhs.m_methodPtr;
        m_callback = rhs.m_callback;

        rhs.m_classPtr = nullptr;
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
        return error<MethodCallbackError>(MethodCallbackError::UNABLE_TO_CALL_METHOD_ON_NULLPTR_CLASS_PTR);
    }

    return internal::ReturnSuccess<ReturnValue>::call(
        m_callback, m_classPtr, m_methodPtr, std::forward<MethodArguments>(args)...);
}

template <typename ReturnValue, typename... Args>
inline bool MethodCallback<ReturnValue, Args...>::operator==(const MethodCallback& rhs) const noexcept
{
    return (m_classPtr == rhs.m_classPtr && m_methodPtr == rhs.m_methodPtr);
}

template <typename ReturnValue, typename... Args>
inline bool MethodCallback<ReturnValue, Args...>::operator!=(const MethodCallback& rhs) const noexcept
{
    return !(this->operator==(rhs));
}

template <typename ReturnValue, typename... Args>
inline bool MethodCallback<ReturnValue, Args...>::isValid() const noexcept
{
    return m_classPtr != nullptr;
}

template <typename ReturnValue, typename... Args>
inline MethodCallback<ReturnValue, Args...>::operator bool() const noexcept
{
    return isValid();
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline void MethodCallback<ReturnValue, Args...>::setObjectPointer(ClassType* const classPtr) noexcept
{
    m_classPtr = classPtr;
}

template <typename ReturnValue, typename... Args>
template <typename ClassType>
inline ClassType* MethodCallback<ReturnValue, Args...>::getClassPointer() const noexcept
{
    return reinterpret_cast<ClassType*>(m_classPtr);
}

} // namespace cxx
} // namespace iox
#endif
