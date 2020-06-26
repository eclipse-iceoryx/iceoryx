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

#ifndef IOX_UTILS_CXX_LIST_INL
#define IOX_UTILS_CXX_LIST_INL


#include "iceoryx_utils/cxx/list.hpp"


namespace iox
{
namespace cxx
{
template <typename T, uint64_t Capacity>
list<T, Capacity>::list() noexcept
{
    init();
}

template <typename T, uint64_t Capacity>
list<T, Capacity>::list(const list& rhs) noexcept
{
    init();
    *this = rhs;
}

template <typename T, uint64_t Capacity>
list<T, Capacity>::list(list&& rhs) noexcept
{
    init();

    auto iter = cbegin();
    for (auto& element : rhs)
    {
        emplace(iter, std::move(element));
    }

    // optional clear(), but prefered to have defined state of source
    rhs.clear();
}


template <typename T, uint64_t Capacity>
list<T, Capacity>& list<T, Capacity>::list::operator=(const list& rhs) noexcept
{
    if (this != &rhs)
    {
        uint64_t i = 0u;
        auto iterThis = begin();
        auto citerRhs = rhs.cbegin();
        auto startSize = size();
        auto minOfLhsRhsSize = std::min(rhs.size(), startSize);

        // copy using copy assignment
        for (; i < minOfLhsRhsSize; ++i)
        {
            *iterThis = *citerRhs;
            ++iterThis;
            ++citerRhs;
        }

        // with rhs.size bigger than this.size: copy further elements from rhs to this
        for (; i < rhs.size(); ++i)
        {
            emplace(cend(), *citerRhs);
            ++citerRhs;
        }

        // with rhs.size smaller than this.size: delete remaining elements of this
        for (; i < startSize; ++i)
        {
            iterThis = erase(iterThis);
        }
    }
    return *this;
}

template <typename T, uint64_t Capacity>
list<T, Capacity>& list<T, Capacity>::list::operator=(list&& rhs) noexcept
{
    if (this != &rhs)
    {
        uint64_t i = 0u;
        auto iterThis = begin();
        auto citerRhs = rhs.begin();
        auto startSize = size();
        auto minOfLhsRhsSize = std::min(rhs.size(), startSize);

        // move using move assignment
        for (; i < minOfLhsRhsSize; ++i)
        {
            *iterThis = std::move(*citerRhs);
            ++iterThis;
            ++citerRhs;
        }

        // with rhs.size bigger than this.size: move construct further elements from rhs to this
        for (; i < rhs.size(); ++i)
        {
            emplace(cend(), std::move(*citerRhs));
            ++citerRhs;
        }

        // with rhs.size smaller than this.size: delete remaining elements of this
        for (; i < startSize; ++i)
        {
            iterThis = erase(iterThis);
        }

        rhs.clear();
    }
    return *this;
}


template <typename T, uint64_t Capacity>
list<T, Capacity>::~list()
{
    clear();
}


template <typename T, uint64_t Capacity>
typename list<T, Capacity>::iterator list<T, Capacity>::begin() noexcept
{
    return iterator{this, getNextIdx(BEGIN_END_LINK_INDEX)};
}
template <typename T, uint64_t Capacity>
typename list<T, Capacity>::const_iterator list<T, Capacity>::begin() const noexcept
{
    return cbegin();
}
template <typename T, uint64_t Capacity>
typename list<T, Capacity>::const_iterator list<T, Capacity>::cbegin() const noexcept
{
    return const_iterator{this, getNextIdx(BEGIN_END_LINK_INDEX)};
}


template <typename T, uint64_t Capacity>
typename list<T, Capacity>::iterator list<T, Capacity>::end() noexcept
{
    return iterator{this, BEGIN_END_LINK_INDEX};
}
template <typename T, uint64_t Capacity>
typename list<T, Capacity>::const_iterator list<T, Capacity>::end() const noexcept
{
    return cend();
}
template <typename T, uint64_t Capacity>
typename list<T, Capacity>::const_iterator list<T, Capacity>::cend() const noexcept
{
    return const_iterator{this, BEGIN_END_LINK_INDEX};
}


template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::empty() const noexcept
{
    return (m_size == 0);
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::full() const noexcept
{
    return (m_size >= Capacity);
}

template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type list<T, Capacity>::size() const noexcept
{
    return m_size;
}

template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type list<T, Capacity>::capacity() const noexcept
{
    return Capacity;
}

template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type list<T, Capacity>::max_size() const noexcept
{
    return capacity();
}


template <typename T, uint64_t Capacity>
template <typename... ConstructorArgs>
T& list<T, Capacity>::emplace_front(ConstructorArgs&&... args) noexcept
{
    return *emplace(cbegin(), std::forward<ConstructorArgs>(args)...);
}

template <typename T, uint64_t Capacity>
template <typename... ConstructorArgs>
T& list<T, Capacity>::emplace_back(ConstructorArgs&&... args) noexcept
{
    return *emplace(cend(), std::forward<ConstructorArgs>(args)...);
}

template <typename T, uint64_t Capacity>
template <typename... ConstructorArgs>
typename list<T, Capacity>::iterator list<T, Capacity>::emplace(const_iterator iter, ConstructorArgs&&... args) noexcept
{
    if (this != iter.m_list)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator of other list can't be used ");
        std::terminate();
    }

    if (m_size >= Capacity)
    {
        errorMessage(__PRETTY_FUNCTION__, " capacity exhausted ");
        return end();
    }


    // unlink from freeList
    const size_type toBeAddedIdx = m_freeListHeadIdx;
    m_freeListHeadIdx = getNextIdx(m_freeListHeadIdx);

    // data class c'tor
    new (getDataPtrFromIdx(toBeAddedIdx)) T(std::forward<ConstructorArgs>(args)...);

    // add to usedList (before iter position)
    setNextIdx(toBeAddedIdx, iter.m_iterListNodeIdx);
    setPrevIdx(toBeAddedIdx, getPrevIdx(iter.m_iterListNodeIdx));
    setNextIdx(getPrevIdx(iter.m_iterListNodeIdx), toBeAddedIdx);
    setPrevIdx(iter.m_iterListNodeIdx, toBeAddedIdx);

    ++m_size;

    return iterator{this, toBeAddedIdx};
}


template <typename T, uint64_t Capacity>
typename list<T, Capacity>::iterator list<T, Capacity>::erase(const_iterator iter) noexcept
{
    if (this != iter.m_list)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator of other list can't be used ");
        std::terminate();
    }

    if (!isValidElementIndex(iter.m_iterListNodeIdx) || empty())
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator is end() or list is empty");
        return end();
    }

    // unlink from usedList
    setPrevIdx(getNextIdx(iter.m_iterListNodeIdx), getPrevIdx(iter.m_iterListNodeIdx));
    setNextIdx(getPrevIdx(iter.m_iterListNodeIdx), getNextIdx(iter.m_iterListNodeIdx));
    size_type ret_iterIdx = getNextIdx(iter.m_iterListNodeIdx);

    // d'tor data class
    (*iter).~T();

    // add to freeList
    setNextIdx(iter.m_iterListNodeIdx, getNextIdx(m_freeListHeadIdx));
    m_freeListHeadIdx = iter.m_iterListNodeIdx;

    --m_size;

    // Iterator to the element following the erased one, or end() if no such element exists.
    return iterator{this, ret_iterIdx};
}

template <typename T, uint64_t Capacity>
typename list<T, Capacity>::size_type list<T, Capacity>::remove(const T& data) noexcept
{
    return remove_if([&](T& this_data) { return this_data == data; });
}


template <typename T, uint64_t Capacity>
template <typename UnaryPredicate>
typename list<T, Capacity>::size_type list<T, Capacity>::remove_if(UnaryPredicate pred) noexcept
{
    size_type removed_cnt = 0;

    auto iter = begin();

    while (iter != cend())
    {
        if (pred(*iter))
        {
            iter = erase(iter);
            ++removed_cnt;
        }
        else
        {
            ++iter;
        }
    }
    return removed_cnt;
}


template <typename T, uint64_t Capacity>
T& list<T, Capacity>::front() noexcept
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
const T& list<T, Capacity>::front() const noexcept
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
T& list<T, Capacity>::back() noexcept
{
    auto iter = end();

    if (!isValidElementIndex((--iter).m_iterListNodeIdx))
    {
        errorMessage(__PRETTY_FUNCTION__, " tried accessing not existing element");
        std::terminate();
    }
    return *iter;
}
template <typename T, uint64_t Capacity>
const T& list<T, Capacity>::back() const noexcept
{
    auto citer = cend();

    if (!isValidElementIndex((--citer).m_iterListNodeIdx))
    {
        errorMessage(__PRETTY_FUNCTION__, " tried accessing not existing element");
        std::terminate();
    }
    return *citer;
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::push_front(const T& data) noexcept
{
    auto sizeBeforePush = m_size;
    emplace(cbegin(), data);
    return ((m_size - 1) == sizeBeforePush);
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::push_front(T&& data) noexcept
{
    auto sizeBeforePush = m_size;
    emplace(cbegin(), std::forward<T>(data));
    return ((m_size - 1) == sizeBeforePush);
}


template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::push_back(const T& data) noexcept
{
    auto sizeBeforePush = m_size;
    emplace(cend(), data);
    return ((m_size - 1) == sizeBeforePush);
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::push_back(T&& data) noexcept
{
    auto sizeBeforePush = m_size;
    emplace(cend(), std::forward<T>(data));
    return ((m_size - 1) == sizeBeforePush);
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::pop_front() noexcept
{
    auto sizeBeforeErase = m_size;
    erase(begin());
    return ((m_size + 1) == sizeBeforeErase);
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::pop_back() noexcept
{
    auto sizeBeforeErase = m_size;
    erase(--end());
    return ((m_size + 1) == sizeBeforeErase);
}

template <typename T, uint64_t Capacity>
typename list<T, Capacity>::iterator list<T, Capacity>::insert(const_iterator citer, const T& data) noexcept
{
    return emplace(citer, data);
}

template <typename T, uint64_t Capacity>
typename list<T, Capacity>::iterator list<T, Capacity>::insert(const_iterator citer, T&& data) noexcept
{
    return emplace(citer, std::forward<T>(data));
}


template <typename T, uint64_t Capacity>
void list<T, Capacity>::clear() noexcept
{
    for (auto iter = cbegin(); iter != cend();)
    {
        iter = erase(iter);
    }
}

/*************************/
// private member functions
/*************************/

/*************************/
// iterator
template <typename T, uint64_t Capacity>
list<T, Capacity>::iterator::iterator(list* parent, size_type idx) noexcept
    : m_list(parent)
    , m_iterListNodeIdx(idx)
{
}

template <typename T, uint64_t Capacity>
typename list<T, Capacity>::iterator& list<T, Capacity>::iterator::operator++() noexcept
{
    // no increment beyond end() / no restart at begin()
    if (m_list->isValidElementIndex(m_iterListNodeIdx))
    {
        m_iterListNodeIdx = m_list->getNextIdx(m_iterListNodeIdx);
    }
    return *this;
}

template <typename T, uint64_t Capacity>
typename list<T, Capacity>::iterator& list<T, Capacity>::iterator::operator--() noexcept
{
    // no increment beyond begin() / no restart at end()
    if (m_list->isValidElementIndex(m_list->getPrevIdx(m_iterListNodeIdx)))
    {
        m_iterListNodeIdx = m_list->getPrevIdx(m_iterListNodeIdx);
    }
    return *this;
}

template <typename T, uint64_t Capacity>
bool list<T, Capacity>::iterator::operator==(const iterator rhs_citer) const noexcept
{
    // break recursive call for iterator::operator== by casting at least one parameter
    return (const_iterator{rhs_citer} == const_iterator{*this});
}
template <typename T, uint64_t Capacity>
bool list<T, Capacity>::iterator::operator==(const const_iterator rhs_citer) const noexcept
{
    return (rhs_citer == const_iterator{*this});
}

template <typename T, uint64_t Capacity>
bool list<T, Capacity>::iterator::operator!=(const iterator rhs_citer) const noexcept
{
    // break recursive call for iterator::operator!= by casting at least one parameter
    return (const_iterator{rhs_citer} != const_iterator{*this});
}
template <typename T, uint64_t Capacity>
bool list<T, Capacity>::iterator::operator!=(const const_iterator rhs_citer) const noexcept
{
    return (rhs_citer != const_iterator{*this});
}

template <typename T, uint64_t Capacity>
T& list<T, Capacity>::iterator::operator*() const noexcept
{
    return *operator->();
} // operator*


template <typename T, uint64_t Capacity>
T* list<T, Capacity>::iterator::operator->() const noexcept
{
    if (!m_list->isValidElementIndex(m_iterListNodeIdx))
    {
        // terminate e.g. when trying to read e.g. from end()
        errorMessage(__PRETTY_FUNCTION__, " malformed iterator");
        std::terminate();
    }

    return m_list->getDataPtrFromIdx(m_iterListNodeIdx);
}
// iterator
/*************************/
// const_iterator

template <typename T, uint64_t Capacity>
list<T, Capacity>::const_iterator::const_iterator(const list* parent, size_type idx) noexcept
    : m_list(parent)
    , m_iterListNodeIdx(idx)
{
}

template <typename T, uint64_t Capacity>
list<T, Capacity>::const_iterator::const_iterator(const iterator& iter) noexcept
    : m_list(iter.m_list)
    , m_iterListNodeIdx(iter.m_iterListNodeIdx)
{
}

template <typename T, uint64_t Capacity>
typename list<T, Capacity>::const_iterator& list<T, Capacity>::const_iterator::operator++() noexcept
{
    // no increment beyond end() / no restart at begin()
    if (m_list->isValidElementIndex(m_iterListNodeIdx))
    {
        m_iterListNodeIdx = m_list->getNextIdx(m_iterListNodeIdx);
    }
    return *this;
}

template <typename T, uint64_t Capacity>
typename list<T, Capacity>::const_iterator& list<T, Capacity>::const_iterator::operator--() noexcept
{
    // no increment beyond begin() / no restart at end()
    if (m_list->isValidElementIndex(m_list->getPrevIdx(m_iterListNodeIdx)))
    {
        m_iterListNodeIdx = m_list->getPrevIdx(m_iterListNodeIdx);
    }
    return *this;
}

template <typename T, uint64_t Capacity>
bool list<T, Capacity>::const_iterator::operator==(const const_iterator rhs_citer) const noexcept
{
    if (this->m_list != rhs_citer.m_list)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterators of different list can't be compared");
        std::terminate();
    }
    // index comparison
    return (m_iterListNodeIdx == rhs_citer.m_iterListNodeIdx);
}

template <typename T, uint64_t Capacity>
bool list<T, Capacity>::const_iterator::operator!=(const const_iterator rhs_citer) const noexcept
{
    return !operator==(rhs_citer);
}


template <typename T, uint64_t Capacity>
const T& list<T, Capacity>::const_iterator::operator*() const noexcept
{
    return *operator->();
} // operator*


template <typename T, uint64_t Capacity>
const T* list<T, Capacity>::const_iterator::operator->() const noexcept
{
    if (!m_list->isValidElementIndex(m_iterListNodeIdx))
    {
        // terminate e.g. when trying to read e.g. from end()
        errorMessage(__PRETTY_FUNCTION__, " malformed iterator");
        std::terminate();
    }
    return m_list->getDataPtrFromIdx(m_iterListNodeIdx);
}


// const_iterator
/*************************/

/*************************/
// init
template <typename T, uint64_t Capacity>
void list<T, Capacity>::init() noexcept
{
    // all list elements are concatenated and accessed via the 'freeListHead'
    // used-list is empty, element-links for used-list are stubbed by BEGIN_END_LINK_INDEX
    setPrevIdx(0U, BEGIN_END_LINK_INDEX);
    setNextIdx(0U, 1U);

    for (size_type i = 1; (i + 1U) < BEGIN_END_LINK_INDEX; ++i)
    {
        setPrevIdx(i, i - 1U);
        setNextIdx(i, i + 1U);
    }
    setPrevIdx(Capacity - 1U, Capacity - 2U);
    setNextIdx(Capacity - 1U, BEGIN_END_LINK_INDEX);
    // reset Heads
    // index '(Capacity)' is the 'end' element of the list which does not have a m_data value (payload), instead
    // it will only provide a link to the first and last _used_-element of the list
    setPrevIdx(Capacity, BEGIN_END_LINK_INDEX);
    setNextIdx(Capacity, BEGIN_END_LINK_INDEX);
    m_freeListHeadIdx = 0U;
} // init


/*************************/
// list-node access

template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type list<T, Capacity>::getPrevIdx(size_type idx) const noexcept
{
    return getLinkPtrFromIdx(idx)->prevIdx;
}
template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type list<T, Capacity>::getNextIdx(size_type idx) const noexcept
{
    return getLinkPtrFromIdx(idx)->nextIdx;
}
template <typename T, uint64_t Capacity>
inline void list<T, Capacity>::setPrevIdx(size_type idx, size_type prevIdx) noexcept
{
    getLinkPtrFromIdx(idx)->prevIdx = prevIdx;
}
template <typename T, uint64_t Capacity>
inline void list<T, Capacity>::setNextIdx(size_type idx, size_type nextIdx) noexcept
{
    getLinkPtrFromIdx(idx)->nextIdx = nextIdx;
}


template <typename T, uint64_t Capacity>
inline T* list<T, Capacity>::getDataPtrFromIdx(size_type idx) const noexcept
{
    if (!isValidElementIndex(idx))
    {
        errorMessage(__PRETTY_FUNCTION__, " malformed index ");
        std::terminate();
    }
    return &(getDataBasePtr()[idx]);
}
template <typename T, uint64_t Capacity>
inline T* list<T, Capacity>::getDataBasePtr() const noexcept
{
    return reinterpret_cast<T*>(const_cast<uint8_t(*)[Capacity][sizeof(T)]>(&m_data));
}

template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::NodeLink* list<T, Capacity>::getLinkPtrFromIdx(size_type idx) const noexcept
{
    if (!isValidIteratorIndex(idx))
    {
        errorMessage(__PRETTY_FUNCTION__, " malformed index ");
        std::terminate();
    }
    return &(getLinkBasePtr()[idx]);
}
template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::NodeLink* list<T, Capacity>::getLinkBasePtr() const noexcept
{
    return reinterpret_cast<NodeLink*>(const_cast<uint8_t(*)[NODE_LINK_COUNT][sizeof(NodeLink)]>(&m_links));
}


template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::isValidIteratorIndex(size_type index) const noexcept
{
    return index < NODE_LINK_COUNT;
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::isValidElementIndex(size_type index) const noexcept
{
    return index < Capacity;
}

/*************************/
// failure handling / messaging

template <typename T, uint64_t Capacity>
inline void list<T, Capacity>::errorMessage(const char* f_source, const char* f_msg) noexcept
{
    std::cerr << f_source << " ::: " << f_msg << std::endl;
}


} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_LIST_INL
