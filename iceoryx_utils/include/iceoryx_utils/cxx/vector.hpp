// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_CXX_VECTOR_HPP
#define IOX_UTILS_CXX_VECTOR_HPP

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iceoryx_utils/cxx/container_storage.hpp>
#include <utility>

namespace iox
{
namespace cxx
{
/// @brief  C++11 compatible vector implementation. We needed to do some
///         adjustments in the API since we do not use exceptions and we require
///         a data structure which can be located fully in the shared memory.
template <typename T, uint64_t Capacity>
class vector : public container_storage<T, Capacity>
{
  public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef value_type const* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef pointer iterator;
    typedef const_pointer const_iterator;
    typedef decltype(Capacity) size_type;

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
    ///         end()
    iterator begin() noexcept;

    /// @brief returns a const iterator to the first element of the vector,
    ///         if the vector is empty it returns the same iterator as
    ///         end()
    const_iterator begin() const noexcept;

    /// @brief returns an iterator to the element which comes after the last
    ///         element (the first element which is outside of the vector).
    iterator end() noexcept;

    /// @brief returns a const iterator to the element which comes after the last
    ///         element (the first element which is outside of the vector).
    const_iterator end() const noexcept;

    /// @brief returns a reference to the first element; terminates if the vector is empty
    /// @return reference to the first element
    reference front() noexcept;

    /// @brief returns a const reference to the first element; terminates if the vector is empty
    /// @return const reference to the first element
    const_reference front() const noexcept;

    /// @brief returns a reference to the last element; terminates if the vector is empty
    /// @return reference to the last element
    reference back() noexcept;

    /// @brief returns a const reference to the last element; terminates if the vector is empty
    /// @return const reference to the last element
    const_reference back() const noexcept;


    /// @brief returns a reference to the element stored at index.
    ///         Terminates, if the element at index does not exist.
    reference at(const uint64_t index) noexcept;

    /// @brief returns a const reference to the element stored at index.
    ///         Terminates, if the element at index does not exist.
    const_reference at(const uint64_t index) const noexcept;

    /// @brief returns a reference to the element stored at index.
    ///         Terminates, if the element at index does not exist.
    reference operator[](const uint64_t index) noexcept;

    /// @brief returns a const reference to the element stored at index.
    ///         Terminates, if the element at index does not exist.
    const_reference operator[](const uint64_t index) const noexcept;

    /// @brief returns true if the vector is empty, otherwise false
    bool empty() const noexcept;

    /// @brief calls the destructor of all contained elements and removes them
    void clear() noexcept;

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

    /// @brief removes an element at the given iterator position. if this element is in
    ///         the middle of the vector every element is moved one place to the
    ///         left to ensure that the elements are stored contiguously.
    ///         Does not throw exceptions iff the underlying data type does not throw on assignment ops.
    ///         "noexcept" omitted intentionally to support 3rd party types with neglected "noexcept".
    /// @return iterator to the next element, returns end() if position is out-of-range
    iterator erase(const_iterator position);

    /// @brief removes an element at the given vector index. if this element is in
    ///         the middle of the vector every element is moved one place to the
    ///         left to ensure that the elements are stored contiguously
    ///         Does not throw exceptions iff the underlying data type does not throw on assignment ops.
    ///         "noexcept" omitted intentionally to support 3rd party types with neglected "noexcept".
    /// @return iterator to the next element, returns end() if index is out-of-range
    iterator erase(const uint64_t index);

  private:
    using element_t = typename uninitialized_array<T, Capacity>::element_t;
};
} // namespace cxx
} // namespace iox

template <typename T, uint64_t CapacityLeft, uint64_t CapacityRight>
bool operator==(const iox::cxx::vector<T, CapacityLeft>& lhs, const iox::cxx::vector<T, CapacityRight>& rhs) noexcept;

template <typename T, uint64_t CapacityLeft, uint64_t CapacityRight>
bool operator!=(const iox::cxx::vector<T, CapacityLeft>& lhs, const iox::cxx::vector<T, CapacityRight>& rhs) noexcept;

#include "iceoryx_utils/internal/cxx/vector.inl"

#endif // IOX_UTILS_CXX_VECTOR_HPP
