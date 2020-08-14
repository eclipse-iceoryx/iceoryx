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

#ifndef IOX_UTILS_CXX_LIST_HPP
#define IOX_UTILS_CXX_LIST_HPP

#include <cstdint>
#include <iostream>

#include "iceoryx_utils/platform/platform_correction.hpp"

namespace iox
{
namespace cxx
{
/// @brief  C++11 compatible bi-directional list implementation.
///         Adjustments in the API were done to not use exceptions and serve the requirement of
///         a data structure movable over shared memory.
///         attempt to add elements to a full list will be ignored.
///         Capacity must at least be 1, (unintended) negative initialization is rejected with compile assertion
///         limitation: concurrency concerns have to be handled by client side.
/// @param T type user data to be managed within list
/// @param Capacity number of maximum list elements a client can push to the list. minimum value is '1'
template <typename T, uint64_t Capacity>
class list
{
  private:
    // forward declarations, private
    struct ListLink;

  public:
    // forward declarations, public
    class const_iterator;

    static_assert(Capacity > 0, "Capacity must be an unsigned integral type >0");

    using value_type = T;
    using size_type = decltype(Capacity);

    /// @brief constructor for an empty list (of T-types elements)
    list() noexcept;

    /// @brief destructs the list and also calls the destructor of all
    ///         contained elements
    ~list();

    /// @brief copy constructor list including elements
    /// @param[in] rhs is the list to copy from (same capacity)
    list(const list& rhs) noexcept;

    /// @brief move constructor list including elements
    /// @param[in] rhs is the list to move-construct elements from (same capacity)
    list(list&& rhs) noexcept;

    /// @brief copy assignment, each element is copied (added) to the constructed list
    ///         any existing elements in 'this'/lhs are removed (same behaviour as std::list : Assigns new contents to
    ///         the container, replacing its current contents, and modifying its size accordingly.)
    /// @param[in] rhs is the list to copy from (same capacity)
    /// @return reference to created list
    list& operator=(const list& rhs) noexcept;

    /// @brief move assignment, list is cleared and initialized, elements are moved from source list
    ///         any existing elements in 'this'/lhs are removed (same behaviour as std::list : Assigns new contents to
    ///         the container, replacing its current contents, and modifying its size accordingly.)
    /// @param[in] rhs is the list to move from ('source', same capacity)
    /// @return reference to created list
    list& operator=(list&& rhs) noexcept;

    /// @brief nested iterator class for list element operations including element access via dereferencing
    ///         iterator may be assigned to different list (non-const pointer to m_List), different list iterators
    ///         however are not compareable

    class iterator
    {
      public:
        // provide the following public types for a std::iterator_traits compatible iterator_category interface
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = void; // so far no difference operations supported
        using pointer = T*;
        using reference = T&;
        // end of iterator_traits interface

        /// @brief prefix increment iterator, so it points to the next list element
        ///         when trying to increment beyond the end of the list,
        ///         iterator stays pointing at the end and a message is forwarded to the error_message
        ///         handler / cerr stream
        /// @return reference to this iterator object
        iterator& operator++() noexcept;

        /// @brief prefix decrement iterator, so it points to the previous list element
        ///         when trying to decrement beyond the beginning of the list,
        ///         iterator stays pointing at the beginning and a message is forwarded to the error_message
        ///         handler / cerr stream
        /// @return reference to this iterator object
        iterator& operator--() noexcept;

        /// @brief comparing list iterators for equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         there is no content for fictional elements at BEGIN_END_LINK_INDEX
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        ///         ADL doesn't find const_iterator::operator== without providing this
        /// @param[in] rhs_citer is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator==(const iterator rhs_citer) const noexcept;

        /// @brief comparing list iterators for equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         there is no content for fictional elements at BEGIN_END_LINK_INDEX
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        /// @param[in] rhs_citer is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator==(const const_iterator rhs_citer) const noexcept;

        /// @brief comparing list iterators for non-equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         there is no content for fictional elements at BEGIN_END_LINK_INDEX
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        ///         ADL doesn't find const_iterator::operator== without providing this
        /// @param[in] rhs_citer is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator!=(const iterator rhs_citer) const noexcept;

        /// @brief comparing list iterators for non-equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         there is no content for fictional elements at BEGIN_END_LINK_INDEX
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        /// @param[in] rhs_citer is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator!=(const const_iterator rhs_citer) const noexcept;

        /// @brief dereferencing element content via iterator-position element
        /// @return reference to list element data
        T& operator*() const noexcept;

        /// @brief dereferencing element content via iterator-position element
        /// @return pointer to list element data
        T* operator->() const noexcept;

      private:
        /// @brief private construct for an iterator, the iterator is bundled to
        ///         an existing parent (object) of type list,
        ///         an iterator is only constructed through calls begin() or end()
        /// @param[in] parent is the list the this iterator operates on
        /// @param[in] idx is the index of the list element (within allocated memory of parent list)
        explicit iterator(list* parent, size_type idx) noexcept;

        friend class list<T, Capacity>;
        list<T, Capacity>* m_list;
        size_type m_iterListNodeIdx;

    }; // class iterator

    /// @brief nested const_iterator class, --> linked data element is 'const'
    class const_iterator
    {
      public:
        // provide the following public types for a std::iterator compatible iterator_category interface
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const T;
        using difference_type = void;
        using pointer = const T*;
        using reference = const T&;


        /// @brief construct a const_iterator from an (non-const_) iterator
        /// @param[in] iter is the iterator which will deliver list and index info for the const_iterator
        const_iterator(const iterator& iter) noexcept;


        /// @brief prefix increment iterator, so it points to the next list element
        ///         when trying to increment beyond the end of the list,
        ///         iterator stays pointing at the end and a message is forwarded to the error_message
        ///         handler / cerr stream
        /// @return reference to this iterator object
        const_iterator& operator++() noexcept;

        /// @brief prefix decrement iterator, so it points to the previous list element
        ///         when trying to decrement beyond the beginning of the list,
        ///         iterator stays pointing at the beginning and a message is forwarded to the error_message
        ///         handler / cerr stream
        /// @return reference to this iterator object
        const_iterator& operator--() noexcept;

        /// @brief comparing list iterators for equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         -> there is no content for fictional elements at BEGIN_END_LINK_INDEX
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        ///         share with between iterator and const_iterator
        /// @param[in] rhs_citer is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator==(const const_iterator rhs_citer) const noexcept;

        /// @brief comparing list iterators for non-equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         -> there is no content for fictional elements at BEGIN_END_LINK_INDEX
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        ///         share with between iterator and const_iterator
        /// @param[in] rhs_citer is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator!=(const const_iterator rhs_citer) const noexcept;

        /// @brief dereferencing element content via iterator-position element
        /// @return reference to list element data
        const T& operator*() const noexcept;

        /// @brief dereferencing element content via iterator-position element
        /// @return pointer to const list data element
        const T* operator->() const noexcept;

      private:
        /// @brief private construct for an iterator, the iterator is bundled to
        ///         an existing parent (object) of type list,
        ///         an iterator is only constructed through calls to begin() or end()
        /// @param[in] parent is the const list the this iterator operates on
        /// @param[in] idx is the index of the list element (within allocated memory of parent list)
        explicit const_iterator(const list* parent, size_type idx) noexcept;

        friend class list<T, Capacity>;
        const list<T, Capacity>* m_list;
        size_type m_iterListNodeIdx;

    }; // class const_iterator

    /// @brief default list operation to retrieve an interator to first list element
    /// @return iterator to first list element, returns iterator to end() when list is empty
    iterator begin() noexcept;

    /// @brief default list operation to retrieve an const_iterator to first list element
    /// @return iterator to first list element, returns iterator to end() when list is empty
    const_iterator begin() const noexcept;

    /// @brief default list operation to retrieve an const_iterator to first list element
    /// @return iterator to first list element, returns iterator to end() when list is empty
    const_iterator cbegin() const noexcept;

    /// @brief default list operation to retrieve an interator to end of list (behind last valid element)
    ///         Terminated when content is attemted to read (operator*, operator->)
    /// @return iterator to end element, does not contain data.
    iterator end() noexcept;

    /// @brief default list operation to retrieve an const_iterator to end of list (behind last valid element)
    ///         Terminated when content is attemted to read (operator*, operator->)
    /// @return iterator to end element, does not contain data.
    const_iterator end() const noexcept;

    /// @brief default list operation to retrieve an const_iterator to end of list (behind last valid element)
    ///         Terminated when content is attemted to read (operator*, operator->)
    /// @return iterator to end element, does not contain data.
    const_iterator cend() const noexcept;

    /// @brief list meta information on filling
    /// @return no elements in list (true), otherwise (false)
    bool empty() const noexcept;

    /// @brief list meta information on filling
    /// @return whether list is full (filled with 'capacity' / 'max_size' elements) (true), otherwise (false)
    bool full() const noexcept;

    /// @brief list meta information on filling
    /// @return current number of elements in list
    /// @min    returns min 0
    /// @max    returns max capacity
    size_type size() const noexcept;

    /// @brief list meta information, maximum number of elements the list can contain
    /// @return list has been initialized with the following number of elements.
    size_type capacity() const noexcept;

    /// @brief list meta information, maximum number of elements the list can contain
    /// @return list has been initialized with the following number of elements, same as capacity()
    size_type max_size() const noexcept;

    /// @brief Returns a reference to the first element in the container.
    ///         calling front() on an empty list will terminate() the processing
    /// @return reference to the first element
    T& front() noexcept;

    /// @brief Returns a reference to the first element in the container.
    ///         calling front() on an empty list will terminate() the processing
    /// @return const reference to the first element
    const T& front() const noexcept;

    /// @brief Returns a reference to the last element in the container.
    ///         calling back() on an empty list will terminate() the processing
    /// @return reference to the last element
    T& back() noexcept;

    /// @brief Returns a reference to the last element in the container.
    ///         calling back() on an empty list will terminate() the processing
    /// @return const reference to the last element
    const T& back() const noexcept;

    /// @brief add element to the beginning of the list
    /// @param[in] data reference to data element
    /// @return successful insertion (true), otherwise no element could be added to list (e.g. full -> false)
    bool push_front(const T& data) noexcept;

    /// @brief create element inplace at the begining of the list
    /// @param[in] data universal reference perfectly forwarded to client class
    /// @return successful insertion (true), otherwise no element could be added to list (e.g. full -> false)
    bool push_front(T&& data) noexcept;

    /// @brief add element to the end of the list
    /// @param[in] data reference to data element
    /// @return successful insertion (true), otherwise no element could be added to list (e.g. full -> false)
    bool push_back(const T& data) noexcept;

    /// @brief create element inplace at the end of the list
    /// @param[in] data universal reference perfectly forwarded to client class
    /// @return successful insertion (true), otherwise no element could be added to list (e.g. full -> false)
    bool push_back(T&& data) noexcept;

    /// @brief remove the first element from the begining of the list
    ///         element destructor will be invoked
    /// @return successful removal (true), otherwise no element could be taken from list (e.g. empty -> false)
    bool pop_front() noexcept;

    /// @brief remove the last element from the end of the list
    ///         element destructor will be invoked
    /// @return successful removal (true), otherwise no element could be taken from list (e.g. empty -> false)
    bool pop_back() noexcept;

    /// @brief remove all elements from the list, list will be empty
    ///         element destructors will be invoked
    void clear() noexcept;

    /// @brief remove next element from linked iterator position
    ///         element destructors will be invoked
    ///         recursive calls to erase_after only delete each 2nd element
    /// @param[in] beforeToBeErasedIter iterator linking the element before the to-be-removed element
    /// @return an (non-const_) iterator to the element after the removed element,
    ///         returns end() element when reached end of list
    iterator erase(const_iterator beforeToBeErasedIter) noexcept;

    /// @brief remove the first element which matches the given comparing element (compare by value)
    ///         requires a the template type T to have operator== defined.
    /// @param[in] data value to compare to
    /// @return the number of elements removed
    size_type remove(const T& data) noexcept;

    /// @brief remove the first element which matches the provided comparison function
    ///         requires a the template type T to have a operator== defined.
    /// @param[in] pred unary predicate which returns ​true if the element should be removed
    /// @return the number of elements removed
    template <typename UnaryPredicate>
    size_type remove_if(UnaryPredicate pred) noexcept;

    /// @brief construct element inplace at begining of list
    /// @param[in] args T-typed construction parameters (initializer list)
    /// @return referene to generated element, return is C++17-conform
    template <typename... ConstructorArgs>
    T& emplace_front(ConstructorArgs&&... args) noexcept;

    /// @brief construct element inplace at end of list
    /// @param[in] args T-typed construction parameters (initializer list)
    /// @return referene to generated element, return is C++17-conform
    template <typename... ConstructorArgs>
    T& emplace_back(ConstructorArgs&&... args) noexcept;

    /// @brief construct element inplace at begining of list
    /// @param[in] args T-typed construction parameters (initializer list)
    /// @param[in] afterToBeEmplacedIter position in list to (construct)insert after
    /// @return iterator to the newly added element
    template <typename... ConstructorArgs>
    iterator emplace(const_iterator afterToBeEmplacedIter, ConstructorArgs&&... args) noexcept;

    /// @brief insert element after iterator position
    /// @param[in] citer iterator with the position to insert after
    /// @param[in] data reference to element to add
    /// @return iterator to the newly added element
    iterator insert(const_iterator citer, const T& data) noexcept;

    /// @brief construct element inplace at begining of list
    /// @param[in] citer iterator with the position to insert after
    /// @param[in] data universal reference perfectly forwarded to client class
    /// @return iterator to the newly added element
    iterator insert(const_iterator citer, T&& data) noexcept;

  private:
    struct NodeLink
    {
        size_type nextIdx;
        size_type prevIdx;
    };

    void init() noexcept;
    T* getDataPtrFromIdx(const size_type idx) const noexcept;
    T* getDataBasePtr() const noexcept;
    NodeLink* getLinkPtrFromIdx(const size_type idx) const noexcept;
    NodeLink* getLinkBasePtr() const noexcept;

    bool isValidIteratorIndex(const size_type idx) const noexcept;
    bool isValidElementIndex(const size_type idx) const noexcept;
    bool invalidElement(const size_type idx) const noexcept;
    bool invalidIterator(const const_iterator& iter) const noexcept;
    bool invalidIterOrDifferentLists(const const_iterator& iter) const noexcept;
    size_type getPrevIdx(const size_type idx) const noexcept;
    size_type getNextIdx(const size_type idx) const noexcept;
    size_type getPrevIdx(const const_iterator& iter) const noexcept;
    size_type getNextIdx(const const_iterator& iter) const noexcept;
    void setPrevIdx(const size_type idx, const size_type prevIdx) noexcept;
    void setNextIdx(const size_type idx, const size_type nextIdx) noexcept;

    static void errorMessage(const char* f_source, const char* f_msg) noexcept;

    //***************************************
    //    members
    //***************************************

    static constexpr size_type BEGIN_END_LINK_INDEX{size_type(Capacity)};
    static constexpr size_type NODE_LINK_COUNT{size_type(Capacity) + 1U};
    static constexpr size_type INVALID_INDEX{size_type(Capacity) + 2U};

    // two member variables point to head of freeList and usedList
    // available elements are moved between freeList and usedList when inserted or removed
    size_type m_freeListHeadIdx{0U};

    // m_links array is one element bigger than request element count. In this additional element links are stored
    // to the beginning and end of the list this additional element (index position 'capacity') contains
    // BEGIN_END_LINK_INDEX to previsous and nect element when list is empty. Otherwise previsous will point to the last
    // valid element and next will point to the first used list element
    using nodelink_t = uint8_t[sizeof(NodeLink)];
    using element_t = uint8_t[sizeof(T)];
    alignas(alignof(T)) nodelink_t m_links[NODE_LINK_COUNT];
    alignas(alignof(T)) element_t m_data[Capacity];

    size_type m_size{0U};
}; // list

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/list.inl"

#endif // IOX_UTILS_CXX_LIST_HPP