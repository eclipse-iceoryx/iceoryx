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

#ifndef IOX_HOOFS_CXX_UNIQUE_PTR_HPP
#define IOX_HOOFS_CXX_UNIQUE_PTR_HPP

#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/requires.hpp"

namespace iox
{
namespace cxx
{
///
/// @brief The unique_ptr class is a heap-less unique ptr implementation, unlike the STL.
/// @tparam[in] T Type to which the unique_ptr is pointing to
/// @tparam[in] D Type of the callable provided as deleter
template <typename T, typename D>
class unique_ptr
{
  public:
    unique_ptr() = delete;

    ///
    /// @brief unique_ptr Creates a unique pointer that takes ownership of an object.
    /// @details A deleter must always be provided as no default can be provided given that no heap is used.
    /// The unique_ptr must know how to delete the managed object when the pointer goes out of scope.
    /// @param object The pointer to the object to be managed.
    /// @param deleter The deleter function for cleaning up the managed object.
    ///
    unique_ptr(T* const object, const function<D>& deleter) noexcept;

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
    /// @return Pointer to managed object or errorHandler call if none owned.
    ///
    T* get() noexcept;

    ///
    /// @brief get Retrieve the underlying raw pointer.
    /// @details The unique_ptr retains ownership, therefore the "borrowed" pointer must not be deleted.
    /// @return Const pointer to managed object or errorHandler call if none owned.
    ///
    const T* get() const noexcept;

    ///
    /// @brief release Release ownership of the underlying pointer.
    /// @param[in] ptrToBeReleased unique_ptr which is destroyed without deleting its underlying data
    /// @return Pointer to the managed object.
    ///
    static T* release(unique_ptr&& ptrToBeReleased) noexcept;

    ///
    /// @brief reset Reset the unique pointer to take ownership of the given pointer.
    /// @details Any previously owned objects will be deleted.
    /// @param ptr Pointer to object to take ownership on. It is forbidden to provide the value nullptr!
    ///
    void reset(T* const ptr) noexcept;

    ///
    /// @brief swap Swaps object ownership with another unique_ptr (incl. deleters)
    /// @param other The unique_ptr with which to swap owned objects.
    ///
    void swap(unique_ptr& other) noexcept;

  private:
    void destroy() noexcept;

  private:
    T* m_ptr = nullptr;
    function<D> m_deleter;
};

/// @brief comparision for two distinct unique_ptr types
/// @tparam T underlying type of lhs
/// @tparam U underlying type of rhs
/// @tparam D type of callable stored as deleter in lhs and rhs
/// @param[in] lhs left side of the comparision
/// @param[in] rhs right side of the comparision
/// @return true if the pointers are equal, otherwise false
template <typename T, typename U, typename D>
bool operator==(const unique_ptr<T, D>& lhs, const unique_ptr<U, D>& rhs) noexcept;

/// @brief inequality check for two distinct unique_ptr types
/// @tparam T underlying type of lhs
/// @tparam U underlying type of rhs
/// @tparam D type of callable stored as deleter in lhs and rhs
/// @param[in] lhs left side of the comparision
/// @param[in] rhs right side of the comparision
/// @return true if the pointers are not equal, otherwise false
template <typename T, typename U, typename D>
bool operator!=(const unique_ptr<T, D>& lhs, const unique_ptr<U, D>& rhs) noexcept;

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/unique_ptr.inl"

#endif // IOX_HOOFS_CXX_UNIQUE_PTR_HPP
