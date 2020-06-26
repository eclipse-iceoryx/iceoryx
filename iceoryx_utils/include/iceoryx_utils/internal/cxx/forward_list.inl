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

#ifndef CXX_FORWARD_LIST_INL_INCLUDED
#define CXX_FORWARD_LIST_INL_INCLUDED

#include <iostream>

#include "iceoryx_utils/cxx/forward_list.hpp"


namespace iox
{
namespace cxx
{
template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>::forward_list() noexcept
{
    init();
}

template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>::forward_list(const forward_list& rhs) noexcept
{
    init();
    *this = rhs;
}

template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>::forward_list(forward_list&& rhs) noexcept
{
    init();
    *this = std::move(rhs);
    rhs.clear();
}


template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>& forward_list<T, CAPACITY>::forward_list::operator=(const forward_list& rhs) noexcept
{
    if (this != &rhs)
    {
        uint64_t i = 0u;
        auto iterThis = before_begin();
        auto citerRhs = rhs.cbefore_begin();
        auto startSize = size();

        // copy using copy assignment
        for (; i < std::min(rhs.size(), size()); ++i)
        {
            ++iterThis;
            ++citerRhs;
            *iterThis = *citerRhs;
        }

        // copy using copy ctor (copy additional elements from rhs.size to this.size)
        for (; i < rhs.size(); ++i)
        {
            ++citerRhs;
            iterThis = emplace_after(iterThis, *citerRhs);
        }

        // delete remaining elements
        for (; i < startSize; ++i)
        {
            erase_after(iterThis);
        }
    }
    return *this;
}

template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>& forward_list<T, CAPACITY>::forward_list::operator=(forward_list&& rhs) noexcept
{
    if (this != &rhs)
    {
        uint64_t i = 0u;
        auto iterThis = before_begin();
        auto iterRhs = rhs.before_begin();
        auto startSize = size();

        // move using move assignment
        for (; i < std::min(rhs.size(), size()); ++i)
        {
            ++iterThis;
            ++iterRhs;
            *iterThis = std::move(*iterRhs);
        }

        // move using move ctor (move additional elements from rhs.size to this.size)
        for (; i < rhs.size(); ++i)
        {
            ++iterRhs;
            iterThis = emplace_after(iterThis, std::move(*iterRhs));
        }

        // delete remaining elements
        for (; i < startSize; ++i)
        {
            erase_after(iterThis);
        }

        rhs.clear();
    }
    return *this;
}


template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>::~forward_list()
{
    clear();
}


template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::iterator forward_list<T, CAPACITY>::before_begin() noexcept
{
    return iterator{this, BEFORE_BEGIN_USED_INDEX};
}
template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::const_iterator forward_list<T, CAPACITY>::before_begin() const noexcept
{
    return cbefore_begin();
}
template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::const_iterator forward_list<T, CAPACITY>::cbefore_begin() const noexcept
{
    return const_iterator{this, BEFORE_BEGIN_USED_INDEX};
}

template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::iterator forward_list<T, CAPACITY>::begin() noexcept
{
    iterator iter = before_begin();
    return ++iter;
}
template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::const_iterator forward_list<T, CAPACITY>::begin() const noexcept
{
    return cbegin();
}
template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::const_iterator forward_list<T, CAPACITY>::cbegin() const noexcept
{
    const_iterator citer = cbefore_begin();
    return ++citer;
}


template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::iterator forward_list<T, CAPACITY>::end() noexcept
{
    return iterator{this, INVALID_INDEX};
}
template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::const_iterator forward_list<T, CAPACITY>::end() const noexcept
{
    return cend();
}
template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::const_iterator forward_list<T, CAPACITY>::cend() const noexcept
{
    return const_iterator{this, INVALID_INDEX};
}


template <typename T, int64_t CAPACITY>
inline bool forward_list<T, CAPACITY>::empty() const noexcept
{
    return (m_size == 0);
}

template <typename T, int64_t CAPACITY>
inline bool forward_list<T, CAPACITY>::full() const noexcept
{
    return (m_size >= CAPACITY);
}

template <typename T, int64_t CAPACITY>
inline typename forward_list<T, CAPACITY>::sizeType forward_list<T, CAPACITY>::size() const noexcept
{
    return m_size;
}

template <typename T, int64_t CAPACITY>
inline typename forward_list<T, CAPACITY>::sizeType forward_list<T, CAPACITY>::capacity() const noexcept
{
    return CAPACITY;
}

template <typename T, int64_t CAPACITY>
inline typename forward_list<T, CAPACITY>::sizeType forward_list<T, CAPACITY>::max_size() const noexcept
{
    return capacity();
}


template <typename T, int64_t CAPACITY>
template <typename... ConstructorArgs>
bool forward_list<T, CAPACITY>::emplace_front(ConstructorArgs&&... args) noexcept
{
    return (cend() != emplace_after(before_begin(), std::forward<ConstructorArgs>(args)...));
}

template <typename T, int64_t CAPACITY>
template <typename... ConstructorArgs>
typename forward_list<T, CAPACITY>::iterator
forward_list<T, CAPACITY>::emplace_after(const_iterator afterToBeEmplacedIter, ConstructorArgs&&... args) noexcept
{
    if (this != afterToBeEmplacedIter.m_cList)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator of other list can't be used ");
        std::terminate();
    }

    const sizeType l_toBeUsedIdx = getHeadFreeEl()->nextIdx;

    if (m_size < CAPACITY && l_toBeUsedIdx < INVALID_INDEX)
    {
        // unlink from freeList
        getHeadFreeEl()->nextIdx = getPointerToListNode()[l_toBeUsedIdx].nextIdx;

        // data class c'tor
        new (&(getPointerToListNode()[l_toBeUsedIdx].data)) T(std::forward<ConstructorArgs>(args)...);

        // add to usedList
        const sizeType emplaceAfterIdx = afterToBeEmplacedIter.m_iterListNodeIdx;
        const sizeType emplaceBeforeIdx = getPointerToListNode()[emplaceAfterIdx].nextIdx;
        getPointerToListNode()[l_toBeUsedIdx].nextIdx = emplaceBeforeIdx;
        getPointerToListNode()[emplaceAfterIdx].nextIdx = l_toBeUsedIdx;

        ++m_size;

        return iterator{const_cast<forward_list<T, CAPACITY>*>(afterToBeEmplacedIter.m_cList), l_toBeUsedIdx};
    }
    else
    {
        errorMessage(__PRETTY_FUNCTION__, " capacity exhausted ");
        return end();
    }
}


template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::iterator
forward_list<T, CAPACITY>::erase_after(const_iterator beforeToBeErasedIter) noexcept
{
    if (this != beforeToBeErasedIter.m_cList)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator of other list can't be used ");
        std::terminate();
    }

    sizeType l_beforeToBeRmIdx = beforeToBeErasedIter.m_iterListNodeIdx;
    if (l_beforeToBeRmIdx >= INVALID_INDEX)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator is end()");
        return end();
    }

    sizeType l_toBeRmIdx = getPointerToListNode()[l_beforeToBeRmIdx].nextIdx;

    if (l_toBeRmIdx < INVALID_INDEX && !empty())
    {
        // unlink from usedList
        getPointerToListNode()[l_beforeToBeRmIdx].nextIdx = getPointerToListNode()[l_toBeRmIdx].nextIdx;

        // data'tor data class
        getPointerToListNode()[l_toBeRmIdx].data.~T();

        // add to freeList
        getPointerToListNode()[l_toBeRmIdx].nextIdx = getHeadFreeEl()->nextIdx;
        getHeadFreeEl()->nextIdx = l_toBeRmIdx;

        --m_size;

        // Iterator to the element following the erased one, or end() if no such element exists.
        return iterator{const_cast<forward_list<T, CAPACITY>*>(beforeToBeErasedIter.m_cList),
                        getPointerToListNode()[l_beforeToBeRmIdx].nextIdx};
    }

    // in case list is empty or iterator reached end()
    return end();
}


template <typename T, int64_t CAPACITY>
bool forward_list<T, CAPACITY>::push_front(const T& ref) noexcept
{
    return emplace_front(ref);
}

template <typename T, int64_t CAPACITY>
bool forward_list<T, CAPACITY>::push_front(T&& uref) noexcept
{
    return emplace_front(std::forward<T>(uref));
}

template <typename T, int64_t CAPACITY>
bool forward_list<T, CAPACITY>::pop_front() noexcept
{
    auto sizeBeforeErase = m_size;
    erase_after(before_begin());
    return ((m_size + 1) == sizeBeforeErase);
}

template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::iterator forward_list<T, CAPACITY>::insert_after(const_iterator citer,
                                                                                     const T& r_data) noexcept
{
    emplace_after(citer, r_data);
}

template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::iterator forward_list<T, CAPACITY>::insert_after(const_iterator citer,
                                                                                     T&& ur_data) noexcept
{
    emplace_after(citer, std::forward<T>(ur_data));
}


template <typename T, int64_t CAPACITY>
void forward_list<T, CAPACITY>::clear()
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
template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>::iterator::iterator(forward_list* parent, sizeType idx) noexcept
    : m_list(parent)
    , m_iterListNodeIdx(idx)
{
}

template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::iterator& forward_list<T, CAPACITY>::iterator::operator++() noexcept
{
    // get node ref from List
    if (m_iterListNodeIdx < INVALID_INDEX)
    {
        listNodePointer const nodePtr = m_list->getNodePtrFromIdx(m_iterListNodeIdx);
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

template <typename T, int64_t CAPACITY>
bool forward_list<T, CAPACITY>::iterator::operator==(const iterator rhs_iter) const noexcept
{
    // break recursive call for iterator::operator== by casting at least one parameter to const_cast
    return (rhs_iter == (const_iterator) * this);
}
template <typename T, int64_t CAPACITY>
bool forward_list<T, CAPACITY>::iterator::operator==(const const_iterator rhs_citer) const noexcept
{
    return (rhs_citer == *this);
}

template <typename T, int64_t CAPACITY>
T& forward_list<T, CAPACITY>::iterator::operator*() const noexcept
{
    if ((m_iterListNodeIdx >= INVALID_INDEX) || (BEFORE_BEGIN_USED_INDEX == m_iterListNodeIdx)
        || (BEFORE_BEGIN_FREE_INDEX == m_iterListNodeIdx))
    {
        // terminate e.g. when trying to read e.g. from end()
        errorMessage(__PRETTY_FUNCTION__, " malformed iterator");
        std::terminate();
    }
    const listNodePointer nodePtr = m_list->getNodePtrFromIdx(m_iterListNodeIdx);
    return nodePtr->data;
} // operator*


template <typename T, int64_t CAPACITY>
T* forward_list<T, CAPACITY>::iterator::operator->() const noexcept
{
    return &operator*();
}
// iterator
/*************************/
// const_iterator

template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>::const_iterator::const_iterator(const forward_list* parent, sizeType idx) noexcept
    : m_cList(parent)
    , m_iterListNodeIdx(idx)
{
}

template <typename T, int64_t CAPACITY>
forward_list<T, CAPACITY>::const_iterator::const_iterator(const iterator iter) noexcept
    : m_cList(iter.m_list)
    , m_iterListNodeIdx(iter.m_iterListNodeIdx)
{
}

template <typename T, int64_t CAPACITY>
typename forward_list<T, CAPACITY>::const_iterator& forward_list<T, CAPACITY>::const_iterator::operator++() noexcept
{
    // get node ref from List
    if (m_iterListNodeIdx < INVALID_INDEX)
    {
        clistNodePointer nodePtr = m_cList->getNodePtrFromIdx(m_iterListNodeIdx);
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


template <typename T, int64_t CAPACITY>
bool forward_list<T, CAPACITY>::const_iterator::operator==(const const_iterator rhs_citer) const noexcept
{
    if (this->m_cList != rhs_citer.m_cList)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterators of different list can't be compared");
        std::terminate();
    }
    // index comparison, not <T>::operator==        // PRQA S 1051
    return (m_iterListNodeIdx == rhs_citer.m_iterListNodeIdx);
}


template <typename T, int64_t CAPACITY>
const T& forward_list<T, CAPACITY>::const_iterator::operator*() const noexcept
{
    if ((m_iterListNodeIdx >= INVALID_INDEX) || (BEFORE_BEGIN_USED_INDEX == m_iterListNodeIdx)
        || (BEFORE_BEGIN_FREE_INDEX == m_iterListNodeIdx))
    {
        // terminate e.g. when trying to read e.g. from end()
        errorMessage(__PRETTY_FUNCTION__, " malformed iterator");
        std::terminate();
    }
    clistNodePointer nodePtr = m_cList->getNodePtrFromIdx(m_iterListNodeIdx);
    return nodePtr->data;
} // operator*


template <typename T, int64_t CAPACITY>
const T* forward_list<T, CAPACITY>::const_iterator::operator->() const noexcept
{
    return &operator*();
}


// const_iterator
/*************************/

/*************************/
// init
template <typename T, int64_t CAPACITY>
void forward_list<T, CAPACITY>::init() noexcept
{
    // initial link empty-list elements
    getPointerToListNode()[BEFORE_BEGIN_USED_INDEX].nextIdx = INVALID_INDEX;

    for (sizeType i = 1u; (i + 1u) < FW_LIST_INT_CAPACITY; ++i)
    {
        getPointerToListNode()[i].nextIdx = i + 1;
    }
    getPointerToListNode()[FW_LIST_INT_CAPACITY - 1].nextIdx = INVALID_INDEX;
} // init


/*************************/
// list-node access
template <typename T, int64_t CAPACITY>
inline typename forward_list<T, CAPACITY>::ListNodeType*
forward_list<T, CAPACITY>::getNodePtrFromIdx(sizeType idx) const noexcept
{
    return &(getPointerToListNode()[idx]);
}

template <typename T, int64_t CAPACITY>
inline typename forward_list<T, CAPACITY>::ListNodeType* forward_list<T, CAPACITY>::getHeadFreeEl() const noexcept
{
    return getNodePtrFromIdx(BEFORE_BEGIN_FREE_INDEX);
}

template <typename T, int64_t CAPACITY>
inline typename forward_list<T, CAPACITY>::ListNodeType* forward_list<T, CAPACITY>::getPointerToListNode() const
    noexcept
{
    return reinterpret_cast<ListNodeType* const>(const_cast<uint8_t(*)[CAPACITY + 2][sizeof(ListNodeType)]>(&m_data));
}

template <typename T, int64_t CAPACITY>
inline void forward_list<T, CAPACITY>::errorMessage(const char* f_source, const char* f_msg) noexcept
{
    std::cerr << f_source << " ::: " << f_msg << std::endl;
}

namespace
{
template <typename U, typename V>
bool operator!=(U lhs, V rhs) noexcept
{
    return !(lhs == rhs);
}
} // namespace

} // namespace cxx
} // namespace iox

#endif // CXX_FORWARD_LIST_INL_INCLUDED