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

#ifndef IOX_HOOFS_CXX_METHOD_CALLBACK_HPP
#define IOX_HOOFS_CXX_METHOD_CALLBACK_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/function_ref.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"

namespace iox
{
namespace cxx
{
namespace internal
{
class GenericClass
{
};
} // namespace internal

enum class MethodCallbackError
{
    UNINITIALIZED_CALLBACK
};

template <typename ReturnValue, typename... Args>
class ConstMethodCallback
{
  public:
    template <typename T>
    using ConstMethodPointer = ReturnValue (T::*)(Args...) const;

    ConstMethodCallback() noexcept = default;
    ConstMethodCallback(const ConstMethodCallback& rhs) noexcept = default;
    ConstMethodCallback& operator=(const ConstMethodCallback& rhs) noexcept = default;
    ~ConstMethodCallback() noexcept = default;

    /// @brief Constructs a ConstMethodCallback from a pointer to a specific object
    ///        and a pointer to a method of that class.
    /// @param[in] objectRef const object reference
    /// @param[in] methodPtr pointer to a const method
    template <typename ClassType>
    ConstMethodCallback(const ClassType& objectRef, ConstMethodPointer<ClassType> const methodPtr) noexcept;

    /// @brief Move constructor
    /// @param[in] rhs move origin
    ConstMethodCallback(ConstMethodCallback&& rhs) noexcept;

    /// @brief Move assignment operator
    /// @param[in] rhs move origin
    /// @return reference to this
    ConstMethodCallback& operator=(ConstMethodCallback&& rhs) noexcept;

    /// @brief Calls the method if the ConstMethodCallback is valid, otherwise it
    ///         will return MethodCallbackError::UNINITIALIZED_CALLBACK
    /// @param[in] args... arguments which will be perfectly forwarded to the method
    /// @return If ConstMethodCallback is valid the return value of the method, otherwise
    ///         an error.
    template <typename... MethodArguments>
    expected<ReturnValue, MethodCallbackError> operator()(MethodArguments&&... args) const noexcept;

    /// @brief Comparison operator. Two ConstMethodCallbacks are equal if they have the
    ///        same object pointer and method pointer
    bool operator==(const ConstMethodCallback& rhs) const noexcept;

    /// @brief Inequality operator. Two ConstMethodCallback are not equal if they have
    ///        different object or method pointer
    bool operator!=(const ConstMethodCallback& rhs) const noexcept;

    /// @brief Verifies if the ConstMethodCallback is valid.
    /// @return true if objectRef != nullptr otherwise false
    explicit operator bool() const noexcept;

    /// @brief Verifies if the ConstMethodCallback is valid.
    /// @return true if objectRef != nullptr otherwise false
    bool isValid() const noexcept;

    /// @brief Sets a new callback.
    /// @param[in] objectRef const reference to the object
    /// @param[in] methodPtr pointer to the method
    template <typename ClassType>
    void setCallback(const ClassType& objectRef, ConstMethodPointer<ClassType> methodPtr) noexcept;

    /// @brief Returns object pointer
    template <typename ClassType>
    const ClassType* getObjectPointer() const noexcept;

    /// @brief Returns cond method pointer
    template <typename ClassType>
    auto getMethodPointer() const noexcept -> ConstMethodPointer<ClassType>;

  private:
    const void* m_objectPtr{nullptr};
    ConstMethodPointer<internal::GenericClass> m_methodPtr{nullptr};
    cxx::function_ref<ReturnValue(const void*, ConstMethodPointer<internal::GenericClass>, Args...)> m_callback;
};

template <typename ReturnValue, typename... Args>
class MethodCallback
{
  public:
    template <typename T>
    using MethodPointer = ReturnValue (T::*)(Args...);

    MethodCallback() noexcept = default;
    MethodCallback(const MethodCallback& rhs) noexcept = default;
    MethodCallback& operator=(const MethodCallback& rhs) noexcept = default;
    ~MethodCallback() noexcept = default;

    /// @brief Constructs a MethodCallback from a pointer to a specific object
    ///        and a pointer to a method of that object.
    /// @param[in] objectRef object reference
    /// @param[in] methodPtr pointer to a method
    template <typename ClassType>
    MethodCallback(ClassType& objectRef, MethodPointer<ClassType> methodPtr) noexcept;

    /// @brief Move constructor
    /// @param[in] rhs move origin
    MethodCallback(MethodCallback&& rhs) noexcept;

    /// @brief Move assignment operator
    /// @param[in] rhs move origin
    /// @return reference to this
    MethodCallback& operator=(MethodCallback&& rhs) noexcept;

    /// @brief Calls the method if the MethodCallback is valid, otherwise it
    ///         will return MethodCallbackError::UNINITIALIZED_CALLBACK
    /// @param[in] args... arguments which will be perfectly forwarded to the method
    /// @return If MethodCallback is valid the return value of the method, otherwise
    ///         an error.
    template <typename... MethodArguments>
    expected<ReturnValue, MethodCallbackError> operator()(MethodArguments&&... args) noexcept;

    /// @brief Comparison operator. Two MethodCallbacks are equal if they have the
    ///        same object pointer and method pointer
    bool operator==(const MethodCallback& rhs) const noexcept;

    /// @brief Inequality operator. Two MethodCallbacks are not equal if they have
    ///        different object or method pointer
    bool operator!=(const MethodCallback& rhs) const noexcept;

    /// @brief Verifies if the MethodCallback is valid.
    /// @return true if objectRef != nullptr otherwise false
    explicit operator bool() const noexcept;

    /// @brief Verifies if the MethodCallback is valid.
    /// @return true if objectRef != nullptr otherwise false
    bool isValid() const noexcept;

    /// @brief Sets a new callback.
    /// @param[in] objectRef const reference to the object
    /// @param[in] methodPtr pointer to the method
    template <typename ClassType>
    void setCallback(ClassType& objectRef, MethodPointer<ClassType> methodPtr) noexcept;

    /// @brief Returns objectRef
    template <typename ClassType>
    ClassType* getObjectPointer() const noexcept;

    /// @brief Returns cond method pointer
    template <typename ClassType>
    auto getMethodPointer() const noexcept -> MethodPointer<ClassType>;

  private:
    void* m_objectPtr{nullptr};
    MethodPointer<internal::GenericClass> m_methodPtr{nullptr};
    cxx::function_ref<ReturnValue(void*, MethodPointer<internal::GenericClass>, Args...)> m_callback;
};

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/method_callback.inl"

#endif
