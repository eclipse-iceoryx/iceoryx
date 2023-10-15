// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

namespace iox
{

template <typename T, uint64_t CAPACITY>
inline FixedPositionContainer<T, CAPACITY>::FixedPositionContainer() noexcept
{
    for (IndexType i = 0; i < CAPACITY;)
    {
        m_slots[i].status = SlotStatus::FREE;

        IndexType next = static_cast<IndexType>(i + 1U);
        m_slots[i].next = next;
        i = next;
    }
    m_slots[Index::LAST].next = Index::INVALID;

    m_begin_free = Index::FIRST;
    m_begin_used = Index::INVALID;
}

template <typename T, uint64_t CAPACITY>
inline FixedPositionContainer<T, CAPACITY>::~FixedPositionContainer() noexcept
{
    for (IndexType i = 0; i < CAPACITY; ++i)
    {
        if (m_slots[i].status == SlotStatus::USED)
        {
            m_data[i].~T();
        }
    }
}

template <typename T, uint64_t CAPACITY>
inline void FixedPositionContainer<T, CAPACITY>::clear() noexcept
{
    for (IndexType i = 0; i < CAPACITY;)
    {
        if (m_slots[i].status == SlotStatus::USED)
        {
            m_data[i].~T();
        }

        m_slots[i].status = SlotStatus::FREE;

        IndexType next = static_cast<IndexType>(i + 1U);
        m_slots[i].next = next;
        i = next;
    }
    m_slots[Index::LAST].next = Index::INVALID;

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
    // +---+
    // |S:#| <- slot with status and index of next slot with same status
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
    //   |F:1|F:5|U:3|U:4|U:I|F:I|   |
    //   +---+---+---+---+---+---+ - +
    // after insertion
    //     begin_used
    //     |
    //     |   begin_free
    //     |   |                   Index::INVALID
    //     ᵛ   ᵛ                   ᵛ
    //   +---+---+---+---+---+---+ - +
    //   |U:2|F:5|U:3|U:4|U:I|F:I|   |
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
    //   |U:3|F:2|F:5|U:4|U:I|F:I|   |
    //   +---+---+---+---+---+---+ - +
    // before insertion
    //     begin_used
    //     |
    //     |       begin_free
    //     |       |               Index::INVALID
    //     ᵛ       ᵛ               ᵛ
    //   +---+---+---+---+---+---+ - +
    //   |U:1|U:3|F:5|U:4|U:I|F:I|   |
    //   +---+---+---+---+---+---+ - +

    const auto index = m_begin_free;
    m_begin_free = m_slots[m_begin_free].next;

    new (&m_data[index]) T(std::forward<Targs>(args)...);
    m_slots[index].status = SlotStatus::USED;

    if (index < m_begin_used)
    {
        m_slots[index].next = m_begin_used;
        m_begin_used = index;
    }
    else
    {
        iox::cxx::EnsuresWithMsg(index != 0, "Corruption detected!");
        for (IndexType i = static_cast<IndexType>(index - 1U);; --i)
        {
            if (m_slots[i].status == SlotStatus::USED)
            {
                m_slots[index].next = m_slots[i].next;
                m_slots[i].next = index;
                break;
            }
            iox::cxx::EnsuresWithMsg(i != 0, "Corruption detected!");
        }
    }

    return Iterator{index, *this};
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::erase(const IndexType index) noexcept
{
    iox::cxx::ExpectsWithMsg(index <= Index::LAST, "Index out of range");

    iox::cxx::EnsuresWithMsg(m_slots[index].status == SlotStatus::USED,
                             "Trying to erase from index pointing to an empty slot!");

    const auto it = Iterator{m_slots[index].next, *this};

    // removing data from the container
    //
    // terminology
    //
    // +---+
    // |S:#| <- slot with status and index of next slot with same status
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
    //   |U:2|F:5|U:3|U:4|U:I|F:I|   |
    //   +---+---+---+---+---+---+ - +
    // after removal
    //     begin_free
    //     |
    //     |       begin_used
    //     |       |               Index::INVALID
    //     ᵛ       ᵛ               ᵛ
    //   +---+---+---+---+---+---+ - +
    //   |F:1|F:5|U:3|U:4|U:I|F:I|   |
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
    //   |F:2|U:3|F:5|U:4|U:I|F:I|   |
    //   +---+---+---+---+---+---+ - +
    // after removal
    //     begin_free
    //     |
    //     |           begin_used
    //     |           |           Index::INVALID
    //     ᵛ           ᵛ           ᵛ
    //   +---+---+---+---+---+---+ - +
    //   |F:1|F:2|F:5|U:4|U:I|F:I|   |
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
    //   |F:2|U:3|F:5|U:4|U:I|F:I|   |
    //   +---+---+---+---+---+---+ - +
    // before removal
    //     begin_free
    //     |
    //     |   begin_used
    //     |   |                   Index::INVALID
    //     ᵛ   ᵛ                   ᵛ
    //   +---+---+---+---+---+---+ - +
    //   |F:2|U:4|F:3|F:5|U:I|F:I|   |
    //   +---+---+---+---+---+---+ - +

    m_data[index].~T();
    m_slots[index].status = SlotStatus::FREE;

    auto next_used = m_slots[index].next;
    bool is_removed_from_used_list{false};
    bool is_added_to_free_list{false};

    if (index == m_begin_used)
    {
        m_begin_used = next_used;
        is_removed_from_used_list = true;
    }

    if (index < m_begin_free)
    {
        m_slots[index].next = m_begin_free;
        m_begin_free = index;
        is_added_to_free_list = true;
    }

    if (is_removed_from_used_list && is_added_to_free_list)
    {
        return it;
    }

    iox::cxx::EnsuresWithMsg(index != 0, "Corruption detected! Index cannot be 0 at this location!");
    for (IndexType i = static_cast<IndexType>(index - 1U); !is_removed_from_used_list || !is_added_to_free_list; --i)
    {
        if (!is_removed_from_used_list && m_slots[i].status == SlotStatus::USED)
        {
            m_slots[i].next = next_used;
            is_removed_from_used_list = true;
        }

        if (!is_added_to_free_list && m_slots[i].status == SlotStatus::FREE)
        {
            m_slots[index].next = m_slots[i].next;
            m_slots[i].next = index;
            is_added_to_free_list = true;
        }

        if (i == 0)
        {
            break;
        }
    }
    iox::cxx::EnsuresWithMsg(is_removed_from_used_list && is_added_to_free_list,
                             "Corruption detected! The container is in a corrupt state!");

    return it;
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::erase(const T* ptr) noexcept
{
    iox::cxx::ExpectsWithMsg(ptr != nullptr, "Pointer is a nullptr!");

    const T* const firstElement = &m_data[0];
    iox::cxx::ExpectsWithMsg(ptr >= firstElement, "Pointer pointing out of the container!");

    const auto index = static_cast<IndexType>(ptr - firstElement);
    iox::cxx::ExpectsWithMsg(index <= Index::LAST, "Pointer pointing out of the container!");

    iox::cxx::ExpectsWithMsg(ptr == &m_data[index], "Pointer is not aligned to an element in the container!");

    // NOTE: if the implementation changes from simply forwarding to 'erase(IndexType)' tests need to be written
    return erase(index);
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::Iterator
FixedPositionContainer<T, CAPACITY>::erase(Iterator it) noexcept
{
    iox::cxx::ExpectsWithMsg(it.origins_from(*this), "Iterator belongs to a different container!");

    // NOTE: if the implementation changes from simply forwarding to 'erase(IndexType)' tests need to be written
    return erase(it.to_index());
}

template <typename T, uint64_t CAPACITY>
inline typename FixedPositionContainer<T, CAPACITY>::ConstIterator
FixedPositionContainer<T, CAPACITY>::erase(ConstIterator it) noexcept
{
    iox::cxx::ExpectsWithMsg(it.origins_from(*this), "Iterator belongs to a different container!");

    // NOTE: if the implementation changes from simply forwarding to 'erase(IndexType)' tests need to be written
    return erase(it.to_index());
}

template <typename T, uint64_t CAPACITY>
inline bool FixedPositionContainer<T, CAPACITY>::empty() const noexcept
{
    return m_begin_used >= Index::INVALID;
}

template <typename T, uint64_t CAPACITY>
inline bool FixedPositionContainer<T, CAPACITY>::full() const noexcept
{
    return m_begin_free >= Index::INVALID;
}

template <typename T, uint64_t CAPACITY>
inline uint64_t FixedPositionContainer<T, CAPACITY>::size() const noexcept
{
    uint64_t count{0};
    IndexType pos = m_begin_used;
    // use a for loop to abort at CAPACITY iterations in case the container is corrupted
    for (IndexType i = 0; i < CAPACITY; ++i)
    {
        if (pos > Index::LAST)
        {
            break;
        }
        ++count;
        pos = m_slots[pos].next;
    }
    return count;
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
    if (index > Index::LAST || m_slots[index].status != SlotStatus::USED)
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
