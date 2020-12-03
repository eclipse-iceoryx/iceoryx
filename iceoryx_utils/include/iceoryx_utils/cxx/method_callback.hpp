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

#ifndef IOX_UTILS_CXX_METHOD_CALLBACK_HPP
#define IOX_UTILS_CXX_METHOD_CALLBACK_HPP

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"

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
    UNABLE_TO_CALL_METHOD_ON_NULLPTR_CLASS_PTR
};

template <typename ReturnValue, typename... Args>
class ConstMethodCallback
{
  public:
    ConstMethodCallback() = default;
    ConstMethodCallback(const ConstMethodCallback& rhs) = default;
    ConstMethodCallback& operator=(const ConstMethodCallback& rhs) = default;
    ~ConstMethodCallback() = default;

    /// @brief Constructs a ConstMethodCallback from a pointer to a specific class
    ///        and a pointer to a method of that class.
    /// @param[in] classPtr class pointer
    /// @param[in] methodPtr pointer to a const method
    template <typename ClassType>
    ConstMethodCallback(ClassType* const classPtr, ReturnValue (ClassType::*methodPtr)(Args...) const) noexcept;

    /// @brief Move constructor
    /// @param[in] rhs move origin
    ConstMethodCallback(ConstMethodCallback&& rhs) noexcept;

    /// @brief Move assignment operator
    /// @param[in] rhs move origin
    /// @return reference to this
    ConstMethodCallback& operator=(ConstMethodCallback&& rhs) noexcept;

    /// @brief Calls the method if the ConstMethodCallback is valid, otherwise it
    ///         will return MethodCallbackError::UNABLE_TO_CALL_METHOD_ON_NULLPTR_CLASS_PTR
    /// @param[in] args... arguments which will be perfectly forwarded to the method
    /// @return If ConstMethodCallback is valid the return value of the method, otherwise
    ///         an error.
    template <typename... MethodArguments>
    expected<ReturnValue, MethodCallbackError> operator()(MethodArguments&&... args) const noexcept;

    /// @brief Comparision operator. Two ConstMethodCallbacks are equal if they have the
    ///        same class pointer and method pointer
    bool operator==(const ConstMethodCallback& rhs) const noexcept;

    /// @brief Inequality operator. Two ConstMethodCallback are not equal if they have
    ///        different class or method pointer
    bool operator!=(const ConstMethodCallback& rhs) const noexcept;

    /// @brief Verifies if the ConstMethodCallback is valid.
    /// @return true if classPtr != nullptr otherwise false
    explicit operator bool() const noexcept;

    /// @brief Verifies if the ConstMethodCallback is valid.
    /// @return true if classPtr != nullptr otherwise false
    bool isValid() const noexcept;

    /// @brief Sets a new classPtr. The callback will then call the same method
    ///        on a different class.
    /// @param[in] classPtr class pointer
    template <typename ClassType>
    void setClassPointer(ClassType* const classPtr) noexcept;

    /// @brief Returns classPtr
    template <typename ClassType>
    ClassType* getClassPointer() const noexcept;

  private:
    void* m_classPtr{nullptr};
    ReturnValue (internal::GenericClass::*m_methodPtr)(Args...) const;
    cxx::function_ref<ReturnValue(void*, ReturnValue (internal::GenericClass::*)(Args...) const, Args...)> m_callback;
};

template <typename ReturnValue, typename... Args>
class MethodCallback
{
  public:
    MethodCallback() = default;
    MethodCallback(const MethodCallback& rhs) = default;
    MethodCallback& operator=(const MethodCallback& rhs) = default;
    ~MethodCallback() = default;

    /// @brief Constructs a MethodCallback from a pointer to a specific class
    ///        and a pointer to a method of that class.
    /// @param[in] classPtr class pointer
    /// @param[in] methodPtr pointer to a method
    template <typename ClassType>
    MethodCallback(ClassType* const classPtr, ReturnValue (ClassType::*methodPtr)(Args...)) noexcept;

    /// @brief Move constructor
    /// @param[in] rhs move origin
    MethodCallback(MethodCallback&& rhs) noexcept;

    /// @brief Move assignment operator
    /// @param[in] rhs move origin
    /// @return reference to this
    MethodCallback& operator=(MethodCallback&& rhs) noexcept;

    /// @brief Calls the method if the MethodCallback is valid, otherwise it
    ///         will return MethodCallbackError::UNABLE_TO_CALL_METHOD_ON_NULLPTR_CLASS_PTR
    /// @param[in] args... arguments which will be perfectly forwarded to the method
    /// @return If MethodCallback is valid the return value of the method, otherwise
    ///         an error.
    template <typename... MethodArguments>
    expected<ReturnValue, MethodCallbackError> operator()(MethodArguments&&... args) noexcept;

    /// @brief Comparision operator. Two MethodCallbacks are equal if they have the
    ///        same class pointer and method pointer
    bool operator==(const MethodCallback& rhs) const noexcept;

    /// @brief Inequality operator. Two MethodCallbacks are not equal if they have
    ///        different class or method pointer
    bool operator!=(const MethodCallback& rhs) const noexcept;

    /// @brief Verifies if the MethodCallback is valid.
    /// @return true if classPtr != nullptr otherwise false
    explicit operator bool() const noexcept;

    /// @brief Verifies if the MethodCallback is valid.
    /// @return true if classPtr != nullptr otherwise false
    bool isValid() const noexcept;

    /// @brief Sets a new classPtr. The callback will then call the same method
    ///        on a different class.
    /// @param[in] classPtr class pointer
    template <typename ClassType>
    void setClassPointer(ClassType* const classPtr) noexcept;

    /// @brief Returns classPtr
    template <typename ClassType>
    ClassType* getClassPointer() const noexcept;

  private:
    void* m_classPtr{nullptr};
    ReturnValue (internal::GenericClass::*m_methodPtr)(Args...);
    cxx::function_ref<ReturnValue(void*, ReturnValue (internal::GenericClass::*)(Args...), Args...)> m_callback;
};

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/method_callback.inl"

#endif
