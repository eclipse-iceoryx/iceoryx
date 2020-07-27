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

#ifndef IOX_UTILS_CXX_FORWARD_LIST_INL
#define IOX_UTILS_CXX_FORWARD_LIST_INL


#include "iceoryx_utils/cxx/forward_list.hpp"


namespace iox
{
namespace cxx
{
template <typename T, uint64_t Capacity>
forward_list<T, Capacity>::forward_list() noexcept
{
    init();
}

template <typename T, uint64_t Capacity>
forward_list<T, Capacity>::forward_list(const forward_list& rhs) noexcept
{
    init();
    *this = rhs;
}

template <typename T, uint64_t Capacity>
forward_list<T, Capacity>::forward_list(forward_list&& rhs) noexcept
{
    init();
    *this = std::move(rhs);
}


template <typename T, uint64_t Capacity>
forward_list<T, Capacity>& forward_list<T, Capacity>::forward_list::operator=(const forward_list& rhs) noexcept
{
    if (this != &rhs)
    {
        uint64_t i = 0u;
        auto iterThis = before_begin();
        auto citerRhs = rhs.cbefore_begin();
        auto startSize = size();
        auto minOfLhsRhsSize = std::min(rhs.size(), startSize);

        // copy using copy assignment
        for (; i < minOfLhsRhsSize; ++i)
        {
            ++iterThis;
            ++citerRhs;
            *iterThis = *citerRhs;
        }

        // with rhs.size bigger than this.size: copy further elements from rhs to this
        for (; i < rhs.size(); ++i)
        {
            ++citerRhs;
            iterThis = emplace_after(iterThis, *citerRhs);
        }

        // with rhs.size smaller than this.size: delete remaining elements of this
        for (; i < startSize; ++i)
        {
            erase_after(iterThis);
        }
    }
    return *this;
}

template <typename T, uint64_t Capacity>
forward_list<T, Capacity>& forward_list<T, Capacity>::forward_list::operator=(forward_list&& rhs) noexcept
{
    if (this != &rhs)
    {
        uint64_t i = 0u;
        auto iterThis = before_begin();
        auto iterRhs = rhs.before_begin();
        auto startSize = size();
        auto minOfLhsRhsSize = std::min(rhs.size(), startSize);

        // move using move assignment
        for (; i < minOfLhsRhsSize; ++i)
        {
            ++iterThis;
            ++iterRhs;
            *iterThis = std::move(*iterRhs);
        }

        // with rhs.size bigger than this.size: move construct further elements from rhs to this
        for (; i < rhs.size(); ++i)
        {
            ++iterRhs;
            iterThis = emplace_after(iterThis, std::move(*iterRhs));
        }

        // with rhs.size smaller than this.size: delete remaining elements of this
        for (; i < startSize; ++i)
        {
            erase_after(iterThis);
        }

        rhs.clear();
    }
    return *this;
}


template <typename T, uint64_t Capacity>
forward_list<T, Capacity>::~forward_list()
{
    clear();
}


template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::iterator forward_list<T, Capacity>::before_begin() noexcept
{
    return iterator{this, BEFORE_BEGIN_USED_INDEX};
}
template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::const_iterator forward_list<T, Capacity>::before_begin() const noexcept
{
    return cbefore_begin();
}
template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::const_iterator forward_list<T, Capacity>::cbefore_begin() const noexcept
{
    return const_iterator{this, BEFORE_BEGIN_USED_INDEX};
}

template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::iterator forward_list<T, Capacity>::begin() noexcept
{
    iterator iter = before_begin();
    return ++iter;
}
template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::const_iterator forward_list<T, Capacity>::begin() const noexcept
{
    return cbegin();
}
template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::const_iterator forward_list<T, Capacity>::cbegin() const noexcept
{
    const_iterator citer = cbefore_begin();
    return ++citer;
}


template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::iterator forward_list<T, Capacity>::end() noexcept
{
    return iterator{this, INVALID_INDEX};
}
template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::const_iterator forward_list<T, Capacity>::end() const noexcept
{
    return cend();
}
template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::const_iterator forward_list<T, Capacity>::cend() const noexcept
{
    return const_iterator{this, INVALID_INDEX};
}


template <typename T, uint64_t Capacity>
inline bool forward_list<T, Capacity>::empty() const noexcept
{
    return (m_size == 0);
}

template <typename T, uint64_t Capacity>
inline bool forward_list<T, Capacity>::full() const noexcept
{
    return (m_size >= Capacity);
}

template <typename T, uint64_t Capacity>
inline typename forward_list<T, Capacity>::size_type forward_list<T, Capacity>::size() const noexcept
{
    return m_size;
}

template <typename T, uint64_t Capacity>
inline typename forward_list<T, Capacity>::size_type forward_list<T, Capacity>::capacity() const noexcept
{
    return Capacity;
}

template <typename T, uint64_t Capacity>
inline typename forward_list<T, Capacity>::size_type forward_list<T, Capacity>::max_size() const noexcept
{
    return capacity();
}


template <typename T, uint64_t Capacity>
template <typename... ConstructorArgs>
bool forward_list<T, Capacity>::emplace_front(ConstructorArgs&&... args) noexcept
{
    return (cend() != emplace_after(before_begin(), std::forward<ConstructorArgs>(args)...));
}

template <typename T, uint64_t Capacity>
template <typename... ConstructorArgs>
typename forward_list<T, Capacity>::iterator
forward_list<T, Capacity>::emplace_after(const_iterator afterToBeEmplacedIter, ConstructorArgs&&... args) noexcept
{
    if (this != afterToBeEmplacedIter.m_list)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator of other list can't be used ");
        std::terminate();
    }

    const size_type toBeAddedIdx = getHeadOfFreeElementList()->nextIdx;

    if (m_size < Capacity && toBeAddedIdx < INVALID_INDEX)
    {
        auto newNodePointer = getNodePtrFromIdx(toBeAddedIdx);

        // unlink from freeList
        getHeadOfFreeElementList()->nextIdx = newNodePointer->nextIdx;

        // data class c'tor
        new (&(newNodePointer->data)) T(std::forward<ConstructorArgs>(args)...);

        // add to usedList
        const size_type afterToBeEmplacedIdx = afterToBeEmplacedIter.m_iterListNodeIdx;
        const size_type emplaceBeforeIdx = getNodePtrFromIdx(afterToBeEmplacedIdx)->nextIdx;
        newNodePointer->nextIdx = emplaceBeforeIdx;
        getNodePtrFromIdx(afterToBeEmplacedIdx)->nextIdx = toBeAddedIdx;

        ++m_size;

        return iterator{const_cast<forward_list<T, Capacity>*>(afterToBeEmplacedIter.m_list), toBeAddedIdx};
    }
    else
    {
        errorMessage(__PRETTY_FUNCTION__, " capacity exhausted ");
        return end();
    }
}


template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::iterator
forward_list<T, Capacity>::erase_after(const_iterator beforeToBeErasedIter) noexcept
{
    if (this != beforeToBeErasedIter.m_list)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator of other list can't be used ");
        std::terminate();
    }

    size_type beforeToBeErasedIdx = beforeToBeErasedIter.m_iterListNodeIdx;
    if (!isValidIteratorIndex(beforeToBeErasedIdx))
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator is end()");
        return end();
    }

    size_type toBeErasedIdx = getNodePtrFromIdx(beforeToBeErasedIdx)->nextIdx;

    if (isValidElementIndex(toBeErasedIdx) && !empty())
    {
        auto toErasedNodePointer = getNodePtrFromIdx(toBeErasedIdx);
        // unlink from usedList
        getNodePtrFromIdx(beforeToBeErasedIdx)->nextIdx = toErasedNodePointer->nextIdx;

        // data'tor data class
        toErasedNodePointer->data.~T();

        // add to freeList
        toErasedNodePointer->nextIdx = getHeadOfFreeElementList()->nextIdx;
        getHeadOfFreeElementList()->nextIdx = toBeErasedIdx;

        --m_size;

        // Iterator to the element following the erased one, or end() if no such element exists.
        return iterator{const_cast<forward_list<T, Capacity>*>(beforeToBeErasedIter.m_list),
                        getNodePtrFromIdx(beforeToBeErasedIdx)->nextIdx};
    }

    // in case list is empty or iterator reached end()
    return end();
}

template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::size_type forward_list<T, Capacity>::remove(const T& data) noexcept
{
    return remove_if([&](T& this_data) { return this_data == data; });
}


template <typename T, uint64_t Capacity>
template <typename UnaryPredicate>
typename forward_list<T, Capacity>::size_type forward_list<T, Capacity>::remove_if(UnaryPredicate pred) noexcept
{
    size_type removed_cnt = 0;

    auto it = before_begin();
    auto next_it = begin();
    while (next_it != cend())
    {
        if (pred(*next_it))
        {
            next_it = erase_after(it);
            ++removed_cnt;
        }
        else
        {
            it = next_it;
            ++next_it;
        }
    }
    return removed_cnt;
}


template <typename T, uint64_t Capacity>
T& forward_list<T, Capacity>::front() noexcept
{
    auto iter = begin();

    if (!isValidElementIndex(iter.m_iterListNodeIdx))
    {
        errorMessage(__PRETTY_FUNCTION__, " tried accessing not existing element");
        std::terminate();
    }

    return *iter;
}


template <typename T, uint64_t Capacity>
const T& forward_list<T, Capacity>::front() const noexcept
{
    auto citer = cbegin();

    if (!isValidElementIndex(citer.m_iterListNodeIdx))
    {
        errorMessage(__PRETTY_FUNCTION__, " tried accessing not existing element");
        std::terminate();
    }

    return *citer;
}


template <typename T, uint64_t Capacity>
bool forward_list<T, Capacity>::push_front(const T& r_data) noexcept
{
    return emplace_front(r_data);
}

template <typename T, uint64_t Capacity>
bool forward_list<T, Capacity>::push_front(T&& ur_data) noexcept
{
    return emplace_front(std::forward<T>(ur_data));
}

template <typename T, uint64_t Capacity>
bool forward_list<T, Capacity>::pop_front() noexcept
{
    auto sizeBeforeErase = m_size;
    erase_after(before_begin());
    return ((m_size + 1) == sizeBeforeErase);
}

template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::iterator forward_list<T, Capacity>::insert_after(const_iterator citer,
                                                                                     const T& data) noexcept
{
    return emplace_after(citer, data);
}

template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::iterator forward_list<T, Capacity>::insert_after(const_iterator citer,
                                                                                     T&& data) noexcept
{
    return emplace_after(citer, std::forward<T>(data));
}


template <typename T, uint64_t Capacity>
void forward_list<T, Capacity>::clear() noexcept
{
    for (auto iter = before_begin(); begin() != end();)
    {
        erase_after(iter);
        iter = before_begin();
    }
}

/*************************/
// private member functions
/*************************/

/*************************/
// iterator
template <typename T, uint64_t Capacity>
forward_list<T, Capacity>::iterator::iterator(forward_list* parent, size_type idx) noexcept
    : m_list(parent)
    , m_iterListNodeIdx(idx)
{
}

template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::iterator& forward_list<T, Capacity>::iterator::operator++() noexcept
{
    // get node ref from List
    if (m_list->isValidIteratorIndex(m_iterListNodeIdx))
    {
        ListNodePointer const nodePtr = m_list->getNodePtrFromIdx(m_iterListNodeIdx);
        m_iterListNodeIdx = nodePtr->nextIdx;
        return *this;
    }
    else
    {
        // do not increase beyond end() position (invalid operation), but return as-is
        errorMessage(__PRETTY_FUNCTION__, "invalid iterator or end()");
        return *this;
    }
}

template <typename T, uint64_t Capacity>
bool forward_list<T, Capacity>::iterator::operator==(const iterator rhs_citer) const noexcept
{
    // break recursive call for iterator::operator== by casting at least one parameter
    return (const_iterator{rhs_citer} == const_iterator{*this});
}

template <typename T, uint64_t Capacity>
bool forward_list<T, Capacity>::iterator::operator!=(const iterator rhs_citer) const noexcept
{
    // break recursive call for iterator::operator!= by casting at least one parameter
    return (const_iterator{rhs_citer} != const_iterator{*this});
}

template <typename T, uint64_t Capacity>
T& forward_list<T, Capacity>::iterator::operator*() const noexcept
{
    if (!m_list->isValidElementIndex(m_iterListNodeIdx))
    {
        // terminate e.g. when trying to read e.g. from end()
        errorMessage(__PRETTY_FUNCTION__, " malformed iterator");
        std::terminate();
    }
    const ListNodePointer nodePtr = m_list->getNodePtrFromIdx(m_iterListNodeIdx);
    return nodePtr->data;
} // operator*


template <typename T, uint64_t Capacity>
T* forward_list<T, Capacity>::iterator::operator->() const noexcept
{
    return &operator*();
}
// iterator
/*************************/
// const_iterator

template <typename T, uint64_t Capacity>
forward_list<T, Capacity>::const_iterator::const_iterator(const forward_list* parent, size_type idx) noexcept
    : m_list(parent)
    , m_iterListNodeIdx(idx)
{
}

template <typename T, uint64_t Capacity>
forward_list<T, Capacity>::const_iterator::const_iterator(const iterator& iter) noexcept
    : m_list(iter.m_list)
    , m_iterListNodeIdx(iter.m_iterListNodeIdx)
{
}

template <typename T, uint64_t Capacity>
typename forward_list<T, Capacity>::const_iterator& forward_list<T, Capacity>::const_iterator::operator++() noexcept
{
    // get node ref from List
    if (m_list->isValidIteratorIndex(m_iterListNodeIdx))
    {
        CListNodePointer nodePtr = m_list->getNodePtrFromIdx(m_iterListNodeIdx);
        m_iterListNodeIdx = nodePtr->nextIdx;
        return *this;
    }
    else
    {
        // do not increase beyond end() position (invalid operation), but return as-is
        errorMessage(__PRETTY_FUNCTION__, " invalid iterator or end()");
        return *this;
    }
}


template <typename T, uint64_t Capacity>
const T& forward_list<T, Capacity>::const_iterator::operator*() const noexcept
{
    if (!m_list->isValidElementIndex(m_iterListNodeIdx))
    {
        // terminate e.g. when trying to read e.g. from end()
        errorMessage(__PRETTY_FUNCTION__, " malformed iterator");
        std::terminate();
    }
    CListNodePointer nodePtr = m_list->getNodePtrFromIdx(m_iterListNodeIdx);
    return nodePtr->data;
} // operator*


template <typename T, uint64_t Capacity>
const T* forward_list<T, Capacity>::const_iterator::operator->() const noexcept
{
    return &operator*();
}


// const_iterator
/*************************/

/*************************/
// init
template <typename T, uint64_t Capacity>
void forward_list<T, Capacity>::init() noexcept
{
    // initial link empty-list elements
    getNodePtrFromIdx(BEFORE_BEGIN_USED_INDEX)->nextIdx = INVALID_INDEX;

    for (size_type i = BEFORE_BEGIN_FREE_INDEX; (i + 1U) < INTERNAL_CAPACITY; ++i)
    {
        getNodePtrFromIdx(i)->nextIdx = i + 1U;
    }
    getNodePtrFromIdx(INTERNAL_CAPACITY - 1U)->nextIdx = INVALID_INDEX;
} // init


/*************************/
// list-node access
template <typename T, uint64_t Capacity>
inline typename forward_list<T, Capacity>::ListNode* forward_list<T, Capacity>::getNodePtrFromIdx(size_type idx) const
    noexcept
{
    return &(getNodePtr()[idx]);
}

template <typename T, uint64_t Capacity>
inline typename forward_list<T, Capacity>::ListNode* forward_list<T, Capacity>::getHeadOfFreeElementList() const
    noexcept
{
    return getNodePtrFromIdx(BEFORE_BEGIN_FREE_INDEX);
}

template <typename T, uint64_t Capacity>
inline typename forward_list<T, Capacity>::ListNode* forward_list<T, Capacity>::getNodePtr() const noexcept
{
    return reinterpret_cast<ListNode* const>(const_cast<uint8_t(*)[Capacity + 2U][sizeof(ListNode)]>(&m_data));
}


template <typename T, uint64_t Capacity>
inline bool forward_list<T, Capacity>::isValidIteratorIndex(size_type index) const noexcept
{
    return index < INVALID_INDEX;
}

template <typename T, uint64_t Capacity>
inline bool forward_list<T, Capacity>::isValidElementIndex(size_type index) const noexcept
{
    return (isValidIteratorIndex(index) && index != BEFORE_BEGIN_USED_INDEX && index != BEFORE_BEGIN_FREE_INDEX);
}

template <typename T, uint64_t Capacity>
inline void forward_list<T, Capacity>::errorMessage(const char* f_source, const char* f_msg) noexcept
{
    std::cerr << f_source << " ::: " << f_msg << std::endl;
}


} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_FORWARD_LIST_INL
