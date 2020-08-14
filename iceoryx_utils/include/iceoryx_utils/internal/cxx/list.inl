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
    if (invalidIterOrDifferentLists(iter))
    {
        return end();
    }

    if (m_size >= Capacity)
    {
        errorMessage(__PRETTY_FUNCTION__, " capacity exhausted ");
        return end();
    }


    // unlink from freeList
    const size_type toBeAddedIdx = m_freeListHeadIdx;
    m_freeListHeadIdx = getNextIdx(m_freeListHeadIdx);

    // set valid links / allowing access on element (e.g. getDataPtrFromIdx() )
    setPrevIdx(toBeAddedIdx, getPrevIdx(iter));
    setNextIdx(toBeAddedIdx, iter.m_iterListNodeIdx);

    // data class c'tor
    new (getDataPtrFromIdx(toBeAddedIdx)) T(std::forward<ConstructorArgs>(args)...);

    // add to usedList (before iter position)
    setNextIdx(getPrevIdx(iter), toBeAddedIdx);
    setPrevIdx(iter.m_iterListNodeIdx, toBeAddedIdx);

    ++m_size;

    return iterator{this, toBeAddedIdx};
}


template <typename T, uint64_t Capacity>
typename list<T, Capacity>::iterator list<T, Capacity>::erase(const_iterator iter) noexcept
{
    if (invalidIterOrDifferentLists(iter))
    {
        return end();
    }

    auto eraseIdx = iter.m_iterListNodeIdx;

    // further narrow-down checks
    if (!isValidElementIdx(eraseIdx) || empty())
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator is end() or list is empty");
        return end();
    }

    // unlink from usedList
    size_type retIdx = getNextIdx(iter);
    setPrevIdx(retIdx, getPrevIdx(iter));
    setNextIdx(getPrevIdx(iter), retIdx);

    // d'tor data class
    iter->~T();

    // mark index as unused / invalid via 'previous index' set to INVALID_INDEX while index is handled within freeList
    setPrevIdx(eraseIdx, INVALID_INDEX);
    // add to freeList
    setNextIdx(eraseIdx, m_freeListHeadIdx);
    m_freeListHeadIdx = eraseIdx;

    --m_size;

    // Iterator to the element following the erased one, or end() if no such element exists.
    return iterator{this, retIdx};
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
    handleInvalidElement(iter.m_iterListNodeIdx);
    return *iter;
}
template <typename T, uint64_t Capacity>
const T& list<T, Capacity>::front() const noexcept
{
    auto citer = cbegin();
    handleInvalidElement(citer.m_iterListNodeIdx);
    return *citer;
}

template <typename T, uint64_t Capacity>
T& list<T, Capacity>::back() noexcept
{
    auto iter = end();
    handleInvalidElement((--iter).m_iterListNodeIdx);
    return *iter;
}
template <typename T, uint64_t Capacity>
const T& list<T, Capacity>::back() const noexcept
{
    auto citer = cend();
    handleInvalidElement((--citer).m_iterListNodeIdx);
    return *citer;
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::push_front(const T& data) noexcept
{
    auto sizeBeforePush = m_size;
    if (m_size < Capacity)
    {
        emplace(cbegin(), data);
    }
    return (m_size == ++sizeBeforePush);
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::push_front(T&& data) noexcept
{
    auto sizeBeforePush = m_size;
    if (m_size < Capacity)
    {
        emplace(cbegin(), std::forward<T>(data));
    }
    return (m_size == ++sizeBeforePush);
}


template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::push_back(const T& data) noexcept
{
    auto sizeBeforePush = m_size;
    if (m_size < Capacity)
    {
        emplace(cend(), data);
    }
    return (m_size == ++sizeBeforePush);
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::push_back(T&& data) noexcept
{
    auto sizeBeforePush = m_size;
    if (m_size < Capacity)
    {
        emplace(cend(), std::forward<T>(data));
    }
    return (m_size == ++sizeBeforePush);
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
// iterator

template <typename T, uint64_t Capacity>
template <bool is_const_iter>
list<T, Capacity>::iterator_base<is_const_iter>::iterator_base(parentListPointer parent, size_type idx) noexcept
    : m_list(parent)
    , m_iterListNodeIdx(idx)
{
}

template <typename T, uint64_t Capacity>
template <bool is_const_iter>
list<T, Capacity>::iterator_base<is_const_iter>::iterator_base(const iterator_base<false>& iter)
    : m_list(iter.m_list)
    , m_iterListNodeIdx(iter.m_iterListNodeIdx)
{
}

template <typename T, uint64_t Capacity>
template <bool is_const_iter>
auto list<T, Capacity>::iterator_base<is_const_iter>::operator++() noexcept ->
    typename list<T, Capacity>::template iterator_base<is_const_iter>&
{
    if (!m_list->handleInvalidIterator(*this))
    {
        // no increment beyond end() / no restart at begin()
        if (m_list->isValidElementIdx(m_iterListNodeIdx))
        {
            m_iterListNodeIdx = m_list->getNextIdx(m_iterListNodeIdx);
        }
    }
    return *this;
}

template <typename T, uint64_t Capacity>
template <bool is_const_iter>
auto list<T, Capacity>::iterator_base<is_const_iter>::operator--() noexcept ->
    typename list<T, Capacity>::template iterator_base<is_const_iter>&
{
    if (!m_list->handleInvalidIterator(*this))
    {
        // no increment beyond begin() / no restart at end()
        if (m_list->isValidElementIdx(m_list->getPrevIdx(m_iterListNodeIdx)))
        {
            m_iterListNodeIdx = m_list->getPrevIdx(m_iterListNodeIdx);
        }
    }
    return *this;
}

template <typename T, uint64_t Capacity>
template <bool is_const_iter>
template <bool is_const_iter_other>
bool list<T, Capacity>::iterator_base<is_const_iter>::operator==(
    const list<T, Capacity>::iterator_base<is_const_iter_other>& rhs) const noexcept
{
    if (m_list->invalidIterOrDifferentLists(rhs) || m_list->handleInvalidIterator(*this))
    {
        return false;
    }
    // index comparison
    return (m_iterListNodeIdx == rhs.m_iterListNodeIdx);
}

template <typename T, uint64_t Capacity>
template <bool is_const_iter>
template <bool is_const_iter_other>
bool list<T, Capacity>::iterator_base<is_const_iter>::operator!=(
    const list<T, Capacity>::iterator_base<is_const_iter_other>& rhs) const noexcept
{
    return !operator==(rhs);
}


template <typename T, uint64_t Capacity>
template <bool is_const_iter>
auto list<T, Capacity>::iterator_base<is_const_iter>::operator*() const noexcept ->
    typename list<T, Capacity>::template iterator_base<is_const_iter>::reference
{
    return *operator->();
}


template <typename T, uint64_t Capacity>
template <bool is_const_iter>
auto list<T, Capacity>::iterator_base<is_const_iter>::operator-> () const noexcept ->
    typename list<T, Capacity>::template iterator_base<is_const_iter>::pointer
{
    return m_list->getDataPtrFromIdx(m_iterListNodeIdx);
}
// iterator
/*************************/

/*************************/
// private member functions
/*************************/

/*************************/
// init
template <typename T, uint64_t Capacity>
void list<T, Capacity>::init() noexcept
{
    // all list elements are concatenated and accessed via the 'freeListHead'
    // used-list is empty, element-links for used-list are stubbed by BEGIN_END_LINK_INDEX
    setPrevIdx(0U, INVALID_INDEX);
    setNextIdx(0U, 1U);

    for (size_type i = 1; (i + 1U) < BEGIN_END_LINK_INDEX; ++i)
    {
        setPrevIdx(i, INVALID_INDEX);
        setNextIdx(i, i + 1U);
    }
    setPrevIdx(Capacity - 1U, INVALID_INDEX);
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
inline const typename list<T, Capacity>::size_type& list<T, Capacity>::getPrevIdx(const size_type idx) const noexcept
{
    return m_links[idx].prevIdx;
}
template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type& list<T, Capacity>::getPrevIdx(const size_type idx) noexcept
{
    return const_cast<size_type&>(static_cast<const list<T, Capacity>*>(this)->getPrevIdx(idx));
}

template <typename T, uint64_t Capacity>
inline const typename list<T, Capacity>::size_type& list<T, Capacity>::getNextIdx(const size_type idx) const noexcept
{
    return m_links[idx].nextIdx;
}
template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type& list<T, Capacity>::getNextIdx(const size_type idx) noexcept
{
    return const_cast<size_type&>(static_cast<const list<T, Capacity>*>(this)->getNextIdx(idx));
}

template <typename T, uint64_t Capacity>
inline const typename list<T, Capacity>::size_type& list<T, Capacity>::getPrevIdx(const const_iterator& iter) const
    noexcept
{
    return getPrevIdx(iter.m_iterListNodeIdx);
}
template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type& list<T, Capacity>::getPrevIdx(const const_iterator& iter) noexcept
{
    return getPrevIdx(iter.m_iterListNodeIdx);
}

template <typename T, uint64_t Capacity>
inline const typename list<T, Capacity>::size_type& list<T, Capacity>::getNextIdx(const const_iterator& iter) const
    noexcept
{
    return getNextIdx(iter.m_iterListNodeIdx);
}
template <typename T, uint64_t Capacity>
inline typename list<T, Capacity>::size_type& list<T, Capacity>::getNextIdx(const const_iterator& iter) noexcept
{
    return getNextIdx(iter.m_iterListNodeIdx);
}


template <typename T, uint64_t Capacity>
inline void list<T, Capacity>::setPrevIdx(const size_type idx, const size_type prevIdx) noexcept
{
    m_links[idx].prevIdx = prevIdx;
}
template <typename T, uint64_t Capacity>
inline void list<T, Capacity>::setNextIdx(const size_type idx, const size_type nextIdx) noexcept
{
    m_links[idx].nextIdx = nextIdx;
}


template <typename T, uint64_t Capacity>
inline const T* list<T, Capacity>::getDataPtrFromIdx(const size_type idx) const noexcept
{
    if (handleInvalidElement(idx))
    {
        // errror handling in handleInvalidElement() call
        return nullptr;
    }

    return &(reinterpret_cast<const T*>(&m_data)[idx]);
}

template <typename T, uint64_t Capacity>
inline T* list<T, Capacity>::getDataPtrFromIdx(const size_type idx) noexcept
{
    return const_cast<T*>(static_cast<const list<T, Capacity>*>(this)->getDataPtrFromIdx(idx));
}

/*************************/
// failure handling / messaging

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::isValidElementIdx(const size_type idx) const noexcept
{
    return (idx < Capacity) && (getPrevIdx(idx) < INVALID_INDEX);
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::handleInvalidElement(const size_type idx) const noexcept
{
    // freeList / invalid elements will have the prevIdx set to INVALID_INDEX
    if (isValidElementIdx(idx))
    {
        return false;
    }
    else
    {
        errorMessage(__PRETTY_FUNCTION__, " malformed index ");
        std::terminate();

        return true;
    }
}

// iterator validition on iterator operations or iterators are function arguments
// not coherent-save, as not each single iterator dereferencing operation is checked for validity.
template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::handleInvalidIterator(const const_iterator& iter) const noexcept
{
    // freeList / invalid elements will have the prevIdx set to INVALID_INDEX
    if ((getPrevIdx(iter) < INVALID_INDEX) && (getNextIdx(iter) < INVALID_INDEX)
        && (iter.m_iterListNodeIdx < INVALID_INDEX))
    {
        return false;
    }
    else
    {
        errorMessage(__PRETTY_FUNCTION__, " invalidated iterator ");
        std::terminate();

        return true;
    }
}

template <typename T, uint64_t Capacity>
inline bool list<T, Capacity>::invalidIterOrDifferentLists(const const_iterator& iter) const noexcept
{
    if (handleInvalidIterator(iter))
    {
        return true;
    }
    else if (this != iter.m_list)
    {
        errorMessage(__PRETTY_FUNCTION__, " iterator of other list can't be used ");
        std::terminate();

        return true;
    }
    else
    {
        return false;
    }
}

template <typename T, uint64_t Capacity>
inline void list<T, Capacity>::errorMessage(const char* source, const char* msg) noexcept
{
    std::cerr << source << " ::: " << msg << std::endl;
}


} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_LIST_INL
