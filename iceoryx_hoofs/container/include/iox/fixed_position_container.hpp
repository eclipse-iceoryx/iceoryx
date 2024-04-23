// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_CONTAINER_FIXED_POSITION_CONTAINER_HPP
#define IOX_HOOFS_CONTAINER_FIXED_POSITION_CONTAINER_HPP

#include "iox/algorithm.hpp"
#include "iox/assertions.hpp"
#include "iox/move_and_copy_helper.hpp"
#include "iox/uninitialized_array.hpp"

#include <cstdint>
#include <functional>
#include <type_traits>

namespace iox
{
/// @brief Implementation of a fixed position container. The elements remain at fixed positions in the container, and
/// inserting or removing elements does not change their positions. The container is optimized for efficient iteration
/// over the elements by always having the 'next' index point to the closest element in memory, which prevents
/// unnecessary back-and-forth jumps.
/// @tparam T is the type the container holds
/// @tparam CAPACITY is the maximum number of elements the container can hold
template <typename T, uint64_t CAPACITY>
class FixedPositionContainer final
{
  private:
    enum class IterMutability : uint8_t
    {
        ITER_MUT,
        ITER_CONST
    };

    template <IterMutability>
    class IteratorBase;

  public:
    using ValueType = T;
    using Reference = ValueType&;
    using const_Reference = const ValueType&;
    using Iterator = IteratorBase<IterMutability::ITER_MUT>;
    using ConstIterator = IteratorBase<IterMutability::ITER_CONST>;

    using IndexType = BestFittingType_t<CAPACITY>;

    struct Index
    {
        static constexpr IndexType FIRST{0};
        static constexpr IndexType LAST{CAPACITY - 1};
        static constexpr IndexType INVALID{CAPACITY};
    };

    FixedPositionContainer() noexcept;
    ~FixedPositionContainer() noexcept;

    FixedPositionContainer(const FixedPositionContainer&) noexcept;
    FixedPositionContainer(FixedPositionContainer&&) noexcept;

    FixedPositionContainer& operator=(const FixedPositionContainer&) noexcept;
    FixedPositionContainer& operator=(FixedPositionContainer&&) noexcept;

    /// @brief Clears the container and calls the destructor on all contained elements
    void clear() noexcept;

    /// @brief Inserts a new element at the next free position by copying the data into the container
    /// @param[in] data the data to be inserted into the container
    /// @return iterator pointing to the inserted element or 'end' iterator if the container was full and 'insert'
    /// failed
    Iterator insert(const T& data) noexcept;

    /// @brief Creates a new element at the next free position directly in-place by forwarding all arguments to the
    /// constructor
    /// @tparam Targs template parameter pack for the perfectly forwarded arguments
    /// @param[in] args arguments which are used by the constructor of the newly emplaced element
    /// @return iterator pointing to the emplaced element or 'end' iterator if the container was full and 'emplace'
    /// failed
    template <typename... Targs>
    Iterator emplace(Targs&&... args) noexcept;

    /// @brief Erases the specified element from the container
    /// @param[in] index an index to an element to erase
    /// @return iterator to the element after the removed element or 'end' iterator if the last element was removed
    /// @attention aborts if the index
    ///              - is out of range
    ///              - points to an empty slot
    Iterator erase(const IndexType index) noexcept;

    /// @brief Erases the specified element from the container
    /// @param[in] ptr a pointer to an element to erase
    /// @return iterator to the element after the removed element or 'end' iterator if the last element was removed
    /// @attention aborts if the pointer
    ///              - points outside of the container
    ///              - is not aligned to a slot in the container
    ///              - points to an empty slot
    Iterator erase(const T* ptr) noexcept;

    /// @brief Erases the specified element from the container
    /// @param[in] it an iterator to an element to erase
    /// @return iterator to the element after the removed element or 'end' iterator if the last element was removed
    /// @attention aborts if the iterator
    ///              - is invalid
    ///              - points to a different container
    ///              - points to an empty slot
    Iterator erase(Iterator it) noexcept;

    /// @brief Erases the specified element from the container
    /// @param[in] it an iterator to an element to erase
    /// @return iterator to the element after the removed element or 'end' iterator if the last element was removed
    /// @attention aborts if the iterator
    ///              - is invalid
    ///              - points to a different container
    ///              - points to an empty slot
    ConstIterator erase(ConstIterator it) noexcept;

    /// @brief Checks if the container is empty
    /// @return 'true' if the container is empty, 'false' otherwise
    [[nodiscard]] bool empty() const noexcept;

    /// @brief Checks if the container is full
    /// @return 'true' if the container is full, 'false' otherwise
    [[nodiscard]] bool full() const noexcept;

    /// @brief Get the number of used slots in the container
    /// @return the number of used slots
    [[nodiscard]] uint64_t size() const noexcept;

    /// @brief Get the capacity of the container
    /// @return the number of available slots
    [[nodiscard]] constexpr uint64_t capacity() const noexcept;

    /// @brief Get the iterator to the element pointed to by the index
    /// @param[in] index of the element the for the iterator
    /// @return iterator pointing to the element at index or end iterator if index was out of bounds or index pointed to
    /// an empty slot
    [[nodiscard]] Iterator iter_from_index(const IndexType index);

    /// @brief Get the const iterator to the element pointed to by the index
    /// @param[in] index of the element the for the iterator
    /// @return iterator pointing to the element at index or end iterator if index was out of bounds or index pointed to
    /// an empty slot
    [[nodiscard]] ConstIterator iter_from_index(const IndexType index) const;

    /// @brief Get an iterator pointing to the beginning of the container
    /// @return iterator pointing to the beginning of the container
    [[nodiscard]] Iterator begin() noexcept;

    /// @brief Get a const iterator pointing to the beginning of the container
    /// @return const iterator pointing to the beginning of the container
    [[nodiscard]] ConstIterator begin() const noexcept;

    /// @brief Get a const iterator pointing to the beginning of the container
    /// @return const iterator pointing to the beginning of the container
    [[nodiscard]] ConstIterator cbegin() const noexcept;

    /// @brief Get an iterator pointing to the end of the container
    /// @return iterator pointing to the end of the container
    [[nodiscard]] Iterator end() noexcept;

    /// @brief Get a const iterator pointing to the end of the container
    /// @return const iterator pointing to the end of the container
    [[nodiscard]] ConstIterator end() const noexcept;

    /// @brief Get a const iterator pointing to the end of the container
    /// @return const iterator pointing to the end of the container
    [[nodiscard]] ConstIterator cend() const noexcept;

  private:
    enum class SlotStatus : uint8_t
    {
        FREE,
        USED,
    };

    template <IterMutability ITER_MUTABILITY>
    class IteratorBase
    {
      public:
        using Container = typename std::conditional<ITER_MUTABILITY == IterMutability::ITER_MUT,
                                                    FixedPositionContainer,
                                                    const FixedPositionContainer>::type;
        using Value = typename std::conditional<ITER_MUTABILITY == IterMutability::ITER_MUT, T, const T>::type;

        friend class FixedPositionContainer;

        /// @brief Construct a const iterator from an iterator
        // NOLINTJUSTIFICATION conversion from non const iterator to const iterator follows the STL behavior
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        IteratorBase(const IteratorBase<IterMutability::ITER_MUT>& other) noexcept
            : m_container(other.m_container.get())
            , m_index(other.m_index)
        {
        }

        /// @brief Assigns an iterator to a const iterator
        IteratorBase& operator=(const IteratorBase<IterMutability::ITER_MUT>& rhs) noexcept
        {
            m_container = rhs.m_container.get();
            m_index = rhs.m_index;
            return *this;
        }

        /// @brief Increment the iterator to point to the next element in the container
        /// @return reference to the iterator after the increment
        IteratorBase& operator++() noexcept
        {
            if (m_index <= Index::LAST)
            {
                m_index = m_container.get().m_next[m_index];
            }
            return *this;
        }

        /// @brief Increment the iterator to point to the next element in the container
        /// @return iterator pointing to the value before the increment
        IteratorBase operator++(int) noexcept
        {
            auto ret = *this;
            ++(*this);
            return ret;
        }

        /// @brief Dereference the iterator to access the value it points to
        /// @return reference to the value the iterator points to
        /// @attention aborts if the iterator
        ///              - is an 'end' iterator
        ///              - the slot the iterator point to is not in use
        [[nodiscard]] Value& operator*() const
        {
            IOX_ENFORCE(m_index <= Index::LAST, "Access with invalid index!");
            IOX_ENFORCE(m_container.get().m_status[m_index] == SlotStatus::USED, "Invalid access! Slot not in use!");
            return m_container.get().m_data[m_index];
        }

        /// @brief Access the value pointed to by the iterator using the arrow operator
        /// @return pointer to the value pointed to by the iterator
        /// @attention aborts if the iterator
        ///              - is an 'end' iterator
        ///              - the slot the iterator point to is not in use
        [[nodiscard]] Value* operator->() const
        {
            IOX_ENFORCE(m_index <= Index::LAST, "Access with invalid index!");
            IOX_ENFORCE(m_container.get().m_status[m_index] == SlotStatus::USED, "Invalid access! Slot not in use!");
            return &m_container.get().m_data[m_index];
        }

        /// @brief Get the pointer to the element the iterator points to
        /// @return pointer to the element the iterator points to
        /// @attention aborts if the iterator
        ///              - is an 'end' iterator
        ///              - the slot the iterator point to is not in use
        [[nodiscard]] Value* to_ptr() const
        {
            IOX_ENFORCE(m_index <= Index::LAST, "Access with invalid index!");
            IOX_ENFORCE(m_container.get().m_status[m_index] == SlotStatus::USED, "Invalid access! Slot not in use!");
            return &m_container.get().m_data[m_index];
        }

        /// @brief Get the index of the element the iterator points to
        /// @return index of the element the iterator points to
        /// @attention this can point out of the container in case of the 'end' iterator
        [[nodiscard]] IndexType to_index() const
        {
            return m_index;
        }

        /// @brief Check if the iterator origins from the provided container
        /// @param[in] container to determine the origin of the iterator
        /// @return 'true' if the iterator origins from the provide container, 'false' otherwise
        [[nodiscard]] bool origins_from(const Container& container) const
        {
            return &m_container.get() == &container;
        }

        /// @brief Compares iterators for equality
        /// @return 'true' if iterators are the same, 'false' otherwise
        template <IterMutability RHS_TYPE>
        [[nodiscard]] bool operator==(const IteratorBase<RHS_TYPE>& rhs) const
        {
            return origins_from(rhs.m_container.get()) && (m_index == rhs.m_index);
        }

        /// @brief Compares iterators for non-equality
        /// @return 'true' if iterators are not the same, 'false' otherwise
        template <IterMutability RHS_TYPE>
        [[nodiscard]] bool operator!=(const IteratorBase<RHS_TYPE>& rhs) const
        {
            return !(*this == rhs);
        }

      private:
        IteratorBase(IndexType index, Container& container) noexcept
            : m_container(container)
            , m_index(index)
        {
        }

      private:
        std::reference_wrapper<Container> m_container;
        IndexType m_index;
    };

  private:
    template <MoveAndCopyOperations Opt, typename RhsType>
    void copy_and_move_impl(RhsType&& rhs) noexcept;

  private:
    UninitializedArray<T, CAPACITY> m_data;
    UninitializedArray<SlotStatus, CAPACITY> m_status;
    UninitializedArray<IndexType, CAPACITY> m_next;
    IndexType m_size{0};
    IndexType m_begin_free{Index::FIRST};
    IndexType m_begin_used{Index::INVALID};
};

} // namespace iox

#include "iox/detail/fixed_position_container.inl"

#endif // IOX_HOOFS_CONTAINER_FIXED_POSITION_CONTAINER_HPP
