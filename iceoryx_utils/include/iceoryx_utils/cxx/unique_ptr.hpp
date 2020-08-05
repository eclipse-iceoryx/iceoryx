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

namespace iox
{
namespace cxx
{

///
/// @todo document how it differs to STL
///
template<typename T>
class unique_ptr{
public:

    using ptr_t = T*;

    ///
    /// @brief unique_ptr Creates an empty unique ptr that owns nothing. Can be passed ownership later.
    ///
    unique_ptr(function_ref<void(ptr_t const)> deleter) noexcept;

    ///
    /// @brief unique_ptr Creates a unique pointer that takes ownership of an object.
    /// @details A deleter must always be provided as no heap is used. The unique_ptr needs to know how to delete
    /// the managed object when the reference is out of scope.
    /// @param ptr The raw pointer to the object to be managed.
    /// @param deleter The deleter function for cleaning up the managed object.
    ///
    unique_ptr(ptr_t ptr, const function_ref<void(ptr_t const)> deleter) noexcept;

    ///
    /// @brief unique_ptr Creates an empty unique pointer that points to a specific memory location.
    /// @details The managed object is initially undefined thus must be defined before accessing.
    /// @param allocation The allocation of memory where managed object will reside once created.
    /// @param deleter The deleter function for cleaning up the managed object.
    ///
    unique_ptr(void* allocation, const function_ref<void(ptr_t const)> deleter) noexcept;

    // Not copy-able to ensure uniqueness.
    unique_ptr(const unique_ptr& other) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    // These might need to be specialized.
    unique_ptr(unique_ptr&& rhs) = default;
    unique_ptr& operator=(unique_ptr&& rhs) = default;

    ///
    /// Automatically deletes the owned object on destruction.
    ///
    ~unique_ptr() noexcept;

    /// Dereference the stored pointer.
    T operator*() noexcept;

    /// Return the stored pointer.
    ptr_t operator->() noexcept;

    ///
    /// @brief get Retrieve the underlying raw pointer.
    /// @details The unique_ptr retains ownership, therefore the "borrowed" pointer must not be deleted.
    /// @return Pointer to managed object or nullptr if none owned.
    ///
    ptr_t get() noexcept;

    ///
    /// @brief release Releases ownership of the underlying pointer.
    /// @return Pointer to the managed object or nullptr if none owned.
    ///
    ptr_t release() noexcept;

    ///
    /// @brief reset Reset the unique_ptr instance's owned object to the one given.
    /// @details Any previously owned objects will be deleted.
    /// @param ptr Pointer to object to take ownership on.
    ///
    void reset(ptr_t ptr) noexcept;

    ///
    /// @brief swap Swaps object ownership with another unique_ptr.
    /// @param other The unique_ptr with which to swap owned objects.
    ///
    void swap(unique_ptr& other) noexcept;

private:
    ptr_t m_ptr = nullptr;
    function_ref<void(ptr_t const)> m_deleter;
};


} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/unique_ptr.inl"

#endif // IOX_UTILS_CXX_UNIQUE_PTR_HPP
