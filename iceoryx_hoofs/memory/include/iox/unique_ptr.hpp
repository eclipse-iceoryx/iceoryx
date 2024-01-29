// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_MEMORY_UNIQUE_PTR_HPP
#define IOX_HOOFS_MEMORY_UNIQUE_PTR_HPP

#include "iox/assertions.hpp"
#include "iox/function.hpp"
#include <memory>

namespace iox
{
///
/// @brief The unique_ptr class is a heap-less unique ptr implementation, unlike the STL.
/// @tparam[in] T Type to which the unique_ptr is pointing to
///
/// Also unlike the STL implementation, the deleters are not encoded in the unique_ptr type, allowing unique_ptr
/// instances with different deleters to be stored in the same containers.
///
/// @code
///     #include "iox/unique_ptr.hpp"
///
///     {
///       iox::unique_ptr<MyClass> myPtr(ptrToInt, [&](MyClass* const ptr) {
///         customAllocator.delete(ptr);
///       });
///
///       // Data can be accessed through unique_ptr
///       IOX_LOG(INFO, myPtr->myClassMember);
///
///       // Resetting the unique_ptr, can be performed by calling the move assignment operator
///       myPtr = std::move(uniquePtrToAnotherInt);
///
///     } // deleter is called, when going out of scope
///
/// @endcode
template <typename T>
class unique_ptr final
{
  public:
    unique_ptr() = delete;

    using DeleterType = void(add_const_conditionally_t<T* const, T>);

    ///
    /// @brief unique_ptr Creates a unique pointer that takes ownership of an object.
    /// @details A deleter must always be provided as no default can be provided given that no heap is used.
    /// The unique_ptr must know how to delete the managed object when the pointer goes out of scope.
    /// @param object The pointer to the object to be managed.
    /// @param deleter The deleter function for cleaning up the managed object.
    ///
    unique_ptr(T* const object, const function<DeleterType>& deleter) noexcept;

    unique_ptr(const unique_ptr& other) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;
    unique_ptr(unique_ptr&& rhs) noexcept;
    unique_ptr& operator=(unique_ptr&& rhs) noexcept;

    ///
    /// @brief Automatically deletes the managed object on destruction.
    ///
    ~unique_ptr() noexcept;

    ///
    /// @brief operator -> Transparent access to the managed object.
    /// @return Pointer to the stored object
    ///
    T* operator->() noexcept;

    ///
    /// @brief operator -> Transparent access to the managed object.
    /// @return Const pointer to the stored object
    ///
    const T* operator->() const noexcept;

    ///
    /// @brief get Retrieve the underlying raw pointer.
    /// @details The unique_ptr retains ownership, therefore the "borrowed" pointer must not be deleted.
    /// @return Pointer to managed object
    ///
    T* get() noexcept;

    ///
    /// @brief get Retrieve the underlying raw pointer.
    /// @details The unique_ptr retains ownership, therefore the "borrowed" pointer must not be deleted.
    /// @return Const pointer to managed object
    ///
    const T* get() const noexcept;

    ///
    /// @brief release Release ownership of the underlying pointer.
    /// @param[in] ptrToBeReleased unique_ptr which is destroyed without deleting its underlying data
    /// @return Pointer to the managed object.
    ///
    static T* release(unique_ptr&& ptrToBeReleased) noexcept;

    ///
    /// @brief swap Swaps object ownership with another unique_ptr (incl. deleters)
    /// @param other The unique_ptr with which to swap owned objects.
    ///
    void swap(unique_ptr& other) noexcept;

  private:
    void destroy() noexcept;

  private:
    T* m_ptr{nullptr};
    function<DeleterType> m_deleter;
};

// AXIVION DISABLE STYLE AutosarC++19_03-A13.5.5: Parameters are explicitly not identical to compare two unique_ptr's
// using different types. STL definies them similarly.

/// @brief comparison for two distinct unique_ptr types
/// @tparam T underlying type of lhs
/// @tparam U underlying type of rhs
/// @param[in] lhs left side of the comparison
/// @param[in] rhs right side of the comparison
/// @return true if the pointers are equal, otherwise false
template <typename T, typename U>
bool operator==(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept;

/// @brief inequality check for two distinct unique_ptr types
/// @tparam T underlying type of lhs
/// @tparam U underlying type of rhs
/// @param[in] lhs left side of the comparison
/// @param[in] rhs right side of the comparison
/// @return true if the pointers are not equal, otherwise false
template <typename T, typename U>
bool operator!=(const unique_ptr<T>& lhs, const unique_ptr<U>& rhs) noexcept;

// AXIVION ENABLE STYLE AutosarC++19_03-A13.5.5: See above
} // namespace iox

#include "iox/detail/unique_ptr.inl"

#endif // IOX_HOOFS_MEMORY_UNIQUE_PTR_HPP
