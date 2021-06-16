// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/function_ref.hpp"

namespace iox
{
namespace cxx
{
///
/// @brief The unique_ptr class is a heap-less unique ptr implementation, unlike the STL.
/// @details To avoid using the heap, deleters are not managed by the pointer itself, and instead must be provided as
/// function references ('cxx:function_ref'). The functions must exist at least as long as the pointers that use them.
///
/// Also unlike the STL implementation, the deleters are not encoded in the unique_ptr type, allowing unique_ptr
/// instances with different deleters to be stored in the same containers.
///
template <typename T>
class unique_ptr
{
  public:
    unique_ptr() = delete;

    ///
    /// @brief unique_ptr Creates an empty unique ptr that owns nothing. Can be passed ownership later via reset.
    ///
    unique_ptr(function_ref<void(T*)>&& deleter) noexcept;

    ///
    /// @brief unique_ptr Creates a unique pointer that takes ownership of an object.
    /// @details A deleter must always be provided as no default can be provided given that no heap is used.
    /// The unique_ptr must know how to delete the managed object when the pointer goes out of scope.
    /// @param ptr The raw pointer to the object to be managed.
    /// @param deleter The deleter function for cleaning up the managed object. As cxx:function_ref used for the deleter
    ///                is non-owning the user needs to care about the lifetime of the callable!
    ///
    unique_ptr(T* const ptr, function_ref<void(T*)>&& deleter) noexcept;

    unique_ptr(const unique_ptr& other) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;
    unique_ptr(unique_ptr&& rhs) noexcept;
    unique_ptr& operator=(unique_ptr&& rhs) noexcept;

    ///
    /// Automatically deletes the managed object on destruction.
    ///
    ~unique_ptr() noexcept;


    unique_ptr<T>& operator=(std::nullptr_t) noexcept;

    ///
    /// @brief operator -> Transparent access to the managed object.
    /// @return
    ///
    T* operator->() noexcept;

    ///
    /// @brief operator -> Transparent access to the managed object.
    /// @return
    ///
    const T* operator->() const noexcept;

    ///
    /// @brief operator bool Returns true if it points to something.
    ///
    explicit operator bool() const noexcept;

    ///
    /// @brief get Retrieve the underlying raw pointer.
    /// @details The unique_ptr retains ownership, therefore the "borrowed" pointer must not be deleted.
    /// @return Pointer to managed object or nullptr if none owned.
    ///
    T* get() noexcept;

    ///
    /// @brief get Retrieve the underlying raw pointer.
    /// @details The unique_ptr retains ownership, therefore the "borrowed" pointer must not be deleted.
    /// @return Pointer to managed object or nullptr if none owned.
    ///
    const T* get() const noexcept;

    ///
    /// @brief release Release ownership of the underlying pointer.
    /// @return Pointer to the managed object or nullptr if none owned.
    ///
    T* release() noexcept;

    ///
    /// @brief reset Reset the unique pointer to take ownership of the given pointer.
    /// @details Any previously owned objects will be deleted. If no pointer given then points to nullptr.
    /// @param ptr Pointer to object to take ownership on.
    ///
    void reset(T* const ptr = nullptr) noexcept;

    ///
    /// @brief swap Swaps object ownership with another unique_ptr (incl. deleters)
    /// @param other The unique_ptr with which to swap owned objects.
    ///
    void swap(unique_ptr& other) noexcept;

  private:
    T* m_ptr = nullptr;
    function_ref<void(T* const)> m_deleter;
};

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/unique_ptr.inl"

#endif // IOX_HOOFS_CXX_UNIQUE_PTR_HPP
