// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
// Copyright (c) 2023 by Dennis Liu <dennis48161025@gmail.com>. All rights reserved.
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

#ifndef IOX_DUST_CONTAINER_DETAIL_FIXED_POSITION_CONTAINER_INL
#define IOX_DUST_CONTAINER_DETAIL_FIXED_POSITION_CONTAINER_INL

#include "iox/fixed_position_container.hpp"
#if __cplusplus < 201703L
#include "iox/detail/fixed_position_container_helper.hpp"
#endif

namespace iox
{

template <typename T, uint64_t CAPACITY>
inline FixedPositionContainer<T, CAPACITY>::FixedPositionContainer() noexcept
{
    for (IndexType i = 0; i < CAPACITY;)
    {
        m_status[i] = SlotStatus::FREE;

        IndexType next = static_cast<IndexType>(i + 1U);
        m_next[i] = next;
        i = next;
    }
    m_next[Index::LAST] = Index::INVALID;

    m_begin_free = Index::FIRST;
    m_begin_used = Index::INVALID;
}

template <typename T, uint64_t CAPACITY>
inline FixedPositionContainer<T, CAPACITY>::~FixedPositionContainer() noexcept
{
    for (IndexType i = 0; i < CAPACITY; ++i)
    {
        if (m_status[i] == SlotStatus::USED)
        {
            m_data[i].~T();
        }
    }
}

template <typename T, uint64_t CAPACITY>
inline FixedPositionContainer<T, CAPACITY>::FixedPositionContainer(const FixedPositionContainer& rhs) noexcept
{
    for (IndexType i = 0; i < CAPACITY; ++i)
    {
        m_status[i] = SlotStatus::FREE;
    }

    *this = rhs;
}

template <typename T, uint64_t CAPACITY>
inline FixedPositionContainer<T, CAPACITY>::FixedPositionContainer(FixedPositionContainer&& rhs) noexcept
{
    for (IndexType i = 0; i < CAPACITY; ++i)
    {
        m_status[i] = SlotStatus::FREE;
    }

    *this = std::move(rhs);
}

template <typename T, uint64_t CAPACITY>
inline FixedPositionContainer<T, CAPACITY>&
FixedPositionContainer<T, CAPACITY>::operator=(const FixedPositionContainer& rhs) noexcept
{
    if (this != &rhs)
    {
        copy_and_move_impl(rhs);
    }
    return *this;
}

template <typename T, uint64_t CAPACITY>
inline FixedPositionContainer<T, CAPACITY>&
FixedPositionContainer<T, CAPACITY>::operator=(FixedPositionContainer&& rhs) noexcept
{
    if (this != &rhs)
    {
        copy_and_move_impl(std::move(rhs));

        // clear rhs
        rhs.clear();
    }
    return *this;
}

template <typename T, uint64_t CAPACITY>
template <typename RhsType>
inline void FixedPositionContainer<T, CAPACITY>::copy_and_move_impl(RhsType&& rhs) noexcept
{
    // we already make sure rhs is always passed by std::move() for move case, therefore
    // the result of "decltype(rhs)" is same as "decltype(std::forward<RhsType>(rhs))"
    static_assert(std::is_rvalue_reference<decltype(rhs)>::value
                      || (std::is_lvalue_reference<decltype(rhs)>::value
                          && std::is_const<std::remove_reference_t<decltype(rhs)>>::value),
                  "RhsType must be const lvalue reference or rvalue reference");

    constexpr bool is_move = std::is_rvalue_reference<decltype(rhs)>::value;

    IndexType i{Index::FIRST};
    auto rhs_it = (std::forward<RhsType>(rhs)).begin();

    // transfer src data to destination
    for (; rhs_it.to_index() != Index::INVALID; ++i, ++rhs_it)
    {
        if (m_status[i] == SlotStatus::USED)
        {
#if __cplusplus >= 201703L
            if constexpr (is_move)
            {
                m_data[i] = std::move(*rhs_it);
            }
            else
            {
                m_data[i] = *rhs_it;
            }
#else
            // We introduce a helper struct primarily due to the test case
            // e1cc7c9f-c1b5-4047-811b-004302af5c00. It demands compile-time if-else branching
            // for classes that are either non-copyable or non-moveable.
            // Note: With C++17's 'if constexpr', the need for these helper structs can be eliminated.
            detail::AssignmentHelper<is_move>::assign(m_data[i], detail::MoveHelper<is_move>::move_or_copy(rhs_it));
#endif
        }
        // use ctor to avoid UB for non-initialized free slots
        else
        {
#if __cplusplus >= 201703L
            if constexpr (is_move)
            {
                new (&m_data[i]) T(std::move(*rhs_it));
            }
            else
            {
                new (&m_data[i]) T(*rhs_it);
            }
#else
            // Same as above
            detail::CtorHelper<is_move>::construct(m_data[i], detail::MoveHelper<is_move>::move_or_copy(rhs_it));
#endif
        }
        m_status[i] = SlotStatus::USED;
        m_next[i] = static_cast<IndexType>(i + 1U);
    }

    for (; i < CAPACITY; ++i)
    {
        if (m_status[i] == SlotStatus::USED)
        {
            m_data[i].~T();
        }

        m_status[i] = SlotStatus::FREE;

        IndexType next = static_cast<IndexType>(i + 1U);
        m_next[i] = next;
    }

    m_next[Index::LAST] = Index::INVALID;
    if (!rhs.empty())
    {
        m_next[rhs.m_size - 1] = Index::INVALID;
    }

    m_begin_free = static_cast<IndexType>(rhs.m_size);
    m_begin_used = rhs.empty() ? Index::INVALID : Index::FIRST;
    m_size = rhs.m_size;
}

template <typename T, uint64_t CAPACITY>
inline void FixedPositionContainer<T, CAPACITY>::clear() noexcept
{
    for (IndexType i = 0; i < CAPACITY;)
    {
        if (m_status[i] == SlotStatus::USED)
        {
            m_data[i].~T();
        }

        m_status[i] = SlotStatus::FREE;

        IndexType next = static_cast<IndexType>(i + 1U);
        m_next[i] = next;
        i = next;
    }
    m_next[Index::LAST] = Index::INVALID;

    m_size = 0;
    m_begin_free = Index::FIRST;
    m_begin_used = Index::INVALID;
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::insert(const T& data) noexcept
{
    // NOTE: if the implementation changes from simply forwarding to 'emplace' tests need to be written
    return emplace(data);
}

template <typename T, uint64_t CAPACITY>
template <typename... Targs>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::emplace(Targs&&... args) noexcept
{
    if (full())
    {
        return end();
    }

    // adding data to the container
    //
    // terminology
    //
    //   slot
    //   ᵛ
    // +---+
    // | 0 | <- index of current slot
    // +---+
    // | S | <- current slot status
    // +---+
    // | # | <- index of next slot with same status
    // +---+
    // S: slot status; can be F(free) or U(used)
    // #: index of next slot with same status; can be a number or I(Index::INVALID) if there are no further slots
    //
    // scenario 1 (begin_used is ahead of begin_free)
    // before insertion
    //     begin_free and index to insert
    //     |
    //     |       begin_used
    //     |       |               Index::INVALID
    //     ᵛ       ᵛ               ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | F | F | U | U | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 1 | 5 | 3 | 4 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +
    // after insertion
    //     begin_used
    //     |
    //     |   begin_free
    //     |   |                   Index::INVALID
    //     ᵛ   ᵛ                   ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | U | F | U | U | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 2 | 5 | 3 | 4 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +
    //
    // scenario 2 (begin_free is ahead of begin_used)
    // before insertion
    //     begin_used
    //     |
    //     |   begin_free and index to insert
    //     |   |                   Index::INVALID
    //     ᵛ   ᵛ                   ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | U | F | F | U | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 3 | 2 | 5 | 4 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +
    // after insertion
    //     begin_used
    //     |
    //     |       begin_free
    //     |       |               Index::INVALID
    //     ᵛ       ᵛ               ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | U | U | F | U | U | F |   |  <- status
    //   +---+---+---+---+---+---+ - +
    //   | 1 | 3 | 5 | 4 | I | I |   |  <- next index with same status
    //   +---+---+---+---+---+---+ - +

    const auto index = m_begin_free;
    m_begin_free = m_next[m_begin_free];

    new (&m_data[index]) T(std::forward<Targs>(args)...);
    m_status[index] = SlotStatus::USED;
    ++m_size;

    if (index < m_begin_used)
    {
        m_next[index] = m_begin_used;
        m_begin_used = index;
    }
    else
    {
        IOX_ENSURES_WITH_MSG(index != 0, "Corruption detected!");
        for (IndexType i = static_cast<IndexType>(index - 1U);; --i)
        {
            if (m_status[i] == SlotStatus::USED)
            {
                m_next[index] = m_next[i];
                m_next[i] = index;
                break;
            }
            IOX_ENSURES_WITH_MSG(i != 0, "Corruption detected!");
        }
    }

    return Iterator{index, *this};
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::erase(const IndexType index) noexcept
{
    IOX_EXPECTS_WITH_MSG(index <= Index::LAST, "Index out of range");

    IOX_ENSURES_WITH_MSG(m_status[index] == SlotStatus::USED, "Trying to erase from index pointing to an empty slot!");

    const auto it = Iterator{m_next[index], *this};

    // removing data from the container
    //
    // terminology
    //
    //   slot
    //   ᵛ
    // +---+
    // | 0 | <- index of current slot
    // +---+
    // | S | <- current slot status
    // +---+
    // | # | <- index of next slot with same status
    // +---+
    // S: slot status; can be F(free) or U(used)
    // #: index of next slot with same status; can be a number or I(Index::INVALID) if there are no further slots
    //
    // scenario 1 (remove at begin_used and begin_free is ahead of remove index)
    // before removal
    //     begin_used and index to remove
    //     |
    //     |   begin_free
    //     |   |                   Index::INVALID
    //     ᵛ   ᵛ                   ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | U | F | U | U | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 2 | 5 | 3 | 4 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +
    // after removal
    //     begin_free
    //     |
    //     |       begin_used
    //     |       |               Index::INVALID
    //     ᵛ       ᵛ               ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | F | F | U | U | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 1 | 5 | 3 | 4 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +
    //
    // scenario 2 (remove at begin_used and remove index is ahead of begin_free)
    // before removal
    //     begin_free
    //     |
    //     |   begin_used and index to remove
    //     |   |                   Index::INVALID
    //     ᵛ   ᵛ                   ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | F | U | F | U | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 2 | 3 | 5 | 4 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +
    // after removal
    //     begin_free
    //     |
    //     |           begin_used
    //     |           |           Index::INVALID
    //     ᵛ           ᵛ           ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | F | F | F | U | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 1 | 2 | 5 | 4 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +
    //
    // scenario 3 (remove ahead of begin_used and begin_free)
    // before removal
    //     begin_free
    //     |
    //     |   begin_used
    //     |   |
    //     |   |       index to remove
    //     |   |       |
    //     |   |       |           Index::INVALID
    //     ᵛ   ᵛ       ᵛ           ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | F | U | F | U | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 2 | 3 | 5 | 4 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +
    // after removal
    //     begin_free
    //     |
    //     |   begin_used
    //     |   |                   Index::INVALID
    //     ᵛ   ᵛ                   ᵛ
    //   +---+---+---+---+---+---+ - +
    //   | 0 | 1 | 2 | 3 | 4 | 5 | I |   <- index
    //   +---+---+---+---+---+---+ - +
    //   | F | U | F | F | U | F |   |   <- status
    //   +---+---+---+---+---+---+ - +
    //   | 2 | 4 | 3 | 5 | I | I |   |   <- next index with same status
    //   +---+---+---+---+---+---+ - +

    m_data[index].~T();
    m_status[index] = SlotStatus::FREE;
    --m_size;

    auto next_used = m_next[index];
    bool is_removed_from_used_list{false};
    bool is_added_to_free_list{false};

    if (index == m_begin_used)
    {
        m_begin_used = next_used;
        is_removed_from_used_list = true;
    }

    if (index < m_begin_free)
    {
        m_next[index] = m_begin_free;
        m_begin_free = index;
        is_added_to_free_list = true;
    }

    if (is_removed_from_used_list && is_added_to_free_list)
    {
        return it;
    }

    IOX_ENSURES_WITH_MSG(index != 0, "Corruption detected! Index cannot be 0 at this location!");
    for (IndexType i = static_cast<IndexType>(index - 1U); !is_removed_from_used_list || !is_added_to_free_list; --i)
    {
        if (!is_removed_from_used_list && m_status[i] == SlotStatus::USED)
        {
            m_next[i] = next_used;
            is_removed_from_used_list = true;
        }

        if (!is_added_to_free_list && m_status[i] == SlotStatus::FREE)
        {
            m_next[index] = m_next[i];
            m_next[i] = index;
            is_added_to_free_list = true;
        }

        if (i == 0)
        {
            break;
        }
    }
    IOX_ENSURES_WITH_MSG(is_removed_from_used_list && is_added_to_free_list,
                         "Corruption detected! The container is in a corrupt state!");

    return it;
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::erase(const T* ptr) noexcept
{
    IOX_EXPECTS_WITH_MSG(ptr != nullptr, "Pointer is a nullptr!");

    const T* const firstElement = &m_data[0];
    IOX_EXPECTS_WITH_MSG(ptr >= firstElement, "Pointer pointing out of the container!");

    const auto index = static_cast<IndexType>(ptr - firstElement);
    IOX_EXPECTS_WITH_MSG(index <= Index::LAST, "Pointer pointing out of the container!");

    IOX_EXPECTS_WITH_MSG(ptr == &m_data[index], "Pointer is not aligned to an element in the container!");

    // NOTE: if the implementation changes from simply forwarding to 'erase(IndexType)' tests need to be written
    return erase(index);
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::erase(Iterator it) noexcept
{
    IOX_EXPECTS_WITH_MSG(it.origins_from(*this), "Iterator belongs to a different container!");

    // NOTE: if the implementation changes from simply forwarding to 'erase(IndexType)' tests need to be written
    return erase(it.to_index());
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::ConstIterator
FixedPositionContainer<T, CAPACITY>::erase(ConstIterator it) noexcept
{
    IOX_EXPECTS_WITH_MSG(it.origins_from(*this), "Iterator belongs to a different container!");

    // NOTE: if the implementation changes from simply forwarding to 'erase(IndexType)' tests need to be written
    return erase(it.to_index());
}

template <typename T, uint64_t CAPACITY>
inline bool FixedPositionContainer<T, CAPACITY>::empty() const noexcept
{
    return m_size == 0;
}

template <typename T, uint64_t CAPACITY>
inline bool FixedPositionContainer<T, CAPACITY>::full() const noexcept
{
    return m_begin_free >= Index::INVALID;
}

template <typename T, uint64_t CAPACITY>
inline uint64_t FixedPositionContainer<T, CAPACITY>::size() const noexcept
{
    return m_size;
}

template <typename T, uint64_t CAPACITY>
inline constexpr uint64_t FixedPositionContainer<T, CAPACITY>::capacity() const noexcept
{
    return CAPACITY;
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::iter_from_index(const IndexType index)
{
    const auto it = static_cast<const FixedPositionContainer*>(this)->iter_from_index(index);
    return Iterator(it.to_index(), *this);
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::ConstIterator
FixedPositionContainer<T, CAPACITY>::iter_from_index(const IndexType index) const
{
    if (index > Index::LAST || m_status[index] != SlotStatus::USED)
    {
        return end();
    }
    return ConstIterator(index, *this);
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator FixedPositionContainer<T, CAPACITY>::begin() noexcept
{
    return Iterator{m_begin_used, *this};
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::ConstIterator
FixedPositionContainer<T, CAPACITY>::begin() const noexcept
{
    return ConstIterator{m_begin_used, *this};
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::ConstIterator
FixedPositionContainer<T, CAPACITY>::cbegin() const noexcept
{
    return ConstIterator{m_begin_used, *this};
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator FixedPositionContainer<T, CAPACITY>::end() noexcept
{
    return Iterator{Index::INVALID, *this};
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::ConstIterator
FixedPositionContainer<T, CAPACITY>::end() const noexcept
{
    return ConstIterator{Index::INVALID, *this};
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::ConstIterator
FixedPositionContainer<T, CAPACITY>::cend() const noexcept
{
    return ConstIterator{Index::INVALID, *this};
}
} // namespace iox

#endif // IOX_DUST_CONTAINER_DETAIL_FIXED_POSITION_CONTAINER_INL
