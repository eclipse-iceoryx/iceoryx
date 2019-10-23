// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <utility>

namespace iox
{
namespace cxx
{
/// @brief  C++11 compatible vector implementation. We needed to do some
///         adjustments in the API since we do not use exceptions and we require
///         a data structure which can be located fully in the shared memory.
template <typename T, uint64_t Capacity>
class vector
{
  public:
    using value_type = T;

    using iterator = T*;
    using const_iterator = const T*;

    /// @brief creates an empty vector
    vector() = default;

    /// @brief creates a vector with count copies of elements with value value
    /// @param [in] count is the number copies which are inserted into the vector
    /// @param [in] value is the value which is inserted into the vector
    vector(const uint64_t count, const T& value);

    /// @brief creates a vector with count copies of elements constructed with the default constructor of T
    /// @param [in] count is the number copies which are inserted into the vector
    vector(const uint64_t count);

    /// @brief copy constructor to copy a vector of the same capacity
    vector(const vector& rhs);

    /// @brief move constructor to move a vector of the same capacity
    vector(vector&& rhs);

    /// @brief destructs the vector and also calls the destructor of all
    ///         contained elements
    ~vector();

    /// @brief copy assignment. if the destination vector contains more
    ///         elements than the source the remaining elements will be
    ///         destructed
    vector& operator=(const vector& rhs);

    /// @brief move assignment. if the destination vector contains more
    ///         elements than the source the remaining elements will be
    ///         destructed
    vector& operator=(vector&& rhs);

    /// @brief returns an iterator to the first element of the vector,
    ///         if the vector is empty it returns the same iterator as
    ///         end (the first iterator which is outside of the vector)
    iterator begin();

    /// @brief returns a const iterator to the first element of the vector,
    ///         if the vector is empty it returns the same iterator as
    ///         end (the first iterator which is outside of the vector)
    const_iterator begin() const;

    /// @brief returns an iterator to the element which comes after the last
    ///         element (the first element which is outside of the vector)
    iterator end();

    /// @brief returns a const iterator to the element which comes after the last
    ///         element (the first element which is outside of the vector)
    const_iterator end() const;

    /// @brief return the pointer to the underlying array
    /// @return pointer to underlying array
    T* data() noexcept;

    /// @brief return the const pointer to the underlying array
    /// @return const pointer to underlying array
    const T* data() const noexcept;

    /// @brief returns a reference to the element stored at index. the behavior
    //          is undefined if the element at index does not exist.
    T& at(const uint64_t index);

    /// @brief returns a cost reference to the element stored at index. the
    ///         behavior is undefined if the element at index does not exist.
    const T& at(const uint64_t index) const;

    /// @brief returns a reference to the element stored at index. the behavior
    //          is undefined if the element at index does not exist.
    T& operator[](const uint64_t index);

    /// @brief returns a cost reference to the element stored at index. the
    ///         behavior is undefined if the element at index does not exist.
    const T& operator[](const uint64_t index) const;

    /// @brief returns a reference to the first element; terminates if the vector is empty
    /// @return reference to the first element
    T& front() noexcept;

    /// @brief returns a const reference to the first element; terminates if the vector is empty
    /// @return const reference to the first element
    const T& front() const noexcept;

    /// @brief returns a reference to the last element; terminates if the vector is empty
    /// @return reference to the last element
    T& back() noexcept;

    /// @brief returns a const reference to the last element; terminates if the vector is empty
    /// @return const reference to the last element
    const T& back() const noexcept;

    /// @brief returns the capacity of the vector which was given via the template
    ///         argument
    uint64_t capacity() const;

    /// @brief returns the number of elements which are currently stored in the
    ///         vector
    uint64_t size() const;

    /// @brief returns true if the vector is emtpy, otherwise false
    bool empty() const;

    /// @brief calls the destructor of all contained elements and removes them
    void clear();

    /// @brief forwards all arguments to the constructor of the contained element
    ///         and performs a placement new
    template <typename... Targs>
    bool emplace_back(Targs&&... args);

    /// @brief appends the given element at the end of the vector
    /// @return true if successful, false if vector already full
    bool push_back(const T& value);

    /// @brief appends the given element at the end of the vector
    /// @return true if successful, false if vector already full
    bool push_back(T&& value);

    /// @brief removes the last element of the vector; calling pop_back on an empty container does nothing
    void pop_back();

    /// @brief removes an element at the given position. if this element is in
    ///         the middle of the vector every element is moved one place to the
    ///         left to ensure that the elements are stored contiguously
    iterator erase(iterator position);

  private:
    using element_t = uint8_t[sizeof(T)];
    alignas(alignof(T)) element_t m_data[Capacity];
    uint64_t m_size = 0;
};
} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/vector.inl"
