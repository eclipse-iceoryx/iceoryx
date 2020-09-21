// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_UTILS_CXX_UNIQUE_PTR_HPP
#define IOX_UTILS_CXX_UNIQUE_PTR_HPP

#include "iceoryx_utils/cxx/function_ref.hpp"
#include <functional>

namespace iox
{
namespace cxx
{
///
/// @todo document how it differs to STL
///
template <typename T>
class unique_ptr
{
  public:
    using ptr_t = T*;

    unique_ptr() = delete;

    ///
    /// @brief unique_ptr Creates an empty unique ptr that owns nothing. Can be passed ownership later via reset.
    ///
    unique_ptr(std::function<void(T*)>&& deleter) noexcept;

    ///
    /// @brief unique_ptr Creates a unique pointer that takes ownership of an object.
    /// @details A deleter must always be provided as no default can be provided given that no head is used.
    /// The unique_ptr must know how to delete the managed object when pointer out of scope.
    /// @param ptr The raw pointer to the object to be managed.
    /// @param deleter The deleter function for cleaning up the managed object.
    ///
    unique_ptr(ptr_t ptr, std::function<void(T*)>&& deleter) noexcept;

    unique_ptr(std::nullptr_t) noexcept;

    // Not copy-able to ensure uniqueness.
    unique_ptr(const unique_ptr& other) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr(unique_ptr&& rhs) noexcept;
    unique_ptr& operator=(unique_ptr&& rhs) noexcept;

    ///
    /// Automatically deletes the owned object on destruction.
    ///
    ~unique_ptr() noexcept;

    ///
    /// Return the stored pointer.
    ///
    ptr_t operator->() noexcept;

    ///
    /// @brief operator bool Returns true if it points to something.
    ///
    explicit operator bool() const noexcept;

    ///
    /// @brief get Retrieve the underlying raw pointer.
    /// @details The unique_ptr retains ownership, therefore the "borrowed" pointer must not be deleted.
    /// @return Pointer to managed object or nullptr if none owned.
    ///
    ptr_t get() const noexcept;

    ///
    /// @brief release Release ownership of the underlying pointer.
    /// @return Pointer to the managed object or nullptr if none owned.
    ///
    ptr_t release() noexcept;

    ///
    /// @brief reset Reset the unique pointer to take ownership of the given pointer.
    /// @details Any previously owned objects will be deleted. If no pointer given then points to nullptr.
    /// @param ptr Pointer to object to take ownership on.
    ///
    void reset(ptr_t ptr = nullptr) noexcept;

    ///
    /// @brief swap Swaps object ownership with another unique_ptr.
    /// @param other The unique_ptr with which to swap owned objects.
    ///
    void swap(unique_ptr& other) noexcept;

  private:
    ptr_t m_ptr = nullptr;
    std::function<void(T* const)> m_deleter;
};


} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/unique_ptr.inl"

#endif // IOX_UTILS_CXX_UNIQUE_PTR_HPP
