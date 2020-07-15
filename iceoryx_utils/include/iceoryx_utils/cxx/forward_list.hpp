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

#ifndef CXX_FORWARD_LIST_HPP_INCLUDED
#define CXX_FORWARD_LIST_HPP_INCLUDED

#include <cstdint>
#include <iostream>

#include "iceoryx_utils/platform/platform_correction.hpp"

namespace iox
{
namespace cxx
{
/// @brief  C++11 compatible uni-directional forward list implementation.
///         Adjustments in the API were done to not use exceptions and serve the requirement of
///         a data structure movable over shared memory.
///         attempt to add elements to a full list will be ignored.
///         CAPACITY must at least be 1, (unintended) negative initialization is rejected with compile assertion
///         limitation: concurrency concerns have to be handled by client side.
/// @param T type user data to be managed within list
/// @param CAPACITY number of maximum list elements a client can push to the list. minimum value is '1'
template <typename T, int64_t CAPACITY>
class forward_list
{
  private:
    // forward declarations, private
    struct ListNode;

  public:
    // forward declarations, public
    class const_iterator;

    static_assert(CAPACITY > 0, "CAPACITY must be an unsigned integral type >0");

    using value_type = T;
    using size_type = typename std::make_unsigned<decltype(CAPACITY)>::type;

    /// @brief constructor for an empty list (of T-types elements)
    forward_list() noexcept;

    /// @brief destructs the list and also calls the destructor of all
    ///         contained elements
    ~forward_list();

    /// @brief copy constructor list including elements
    /// @param[in] rhs is the list to copy from (same capacity)
    forward_list(const forward_list& rhs) noexcept;

    /// @brief move constructor list including elements
    /// @param[in] rhs is the list to move-construct elements from (same capacity)
    forward_list(forward_list&& rhs) noexcept;

    /// @brief copy assignment, each element is copied (added) to the constructed list
    ///         any existing elements in 'this'/lhs are removed (same behaviour as std::list : Assigns new contents to
    ///         the container, replacing its current contents, and modifying its size accordingly.)
    /// @param[in] rhs is the list to copy from (same capacity)
    /// @return reference to created list
    forward_list& operator=(const forward_list& rhs) noexcept;

    /// @brief move assignment, list is cleared and initialized, elements are moved from source list
    ///         any existing elements in 'this'/lhs are removed (same behaviour as std::list : Assigns new contents to
    ///         the container, replacing its current contents, and modifying its size accordingly.)
    /// @param[in] rhs is the list to move from ('source', same capacity)
    /// @return reference to created list
    forward_list& operator=(forward_list&& rhs) noexcept;

    /// @brief nested iterator class for list element operations including element access via dereferencing
    ///         iterator may be assigned to different list (non-const pointer to m_List), different list iterators
    ///         however are not compareable
    class iterator
    {
      public:
        // provide the following public types for a std::iterator_traits compatible iterator_category interface
        using iterator_category = std::forward_iterator_tag;
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


        /// @brief comparing list iterators on equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         there is no content for meta-elements like before_begin() and end()
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        /// @param[in] rhs_iter is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator==(const iterator rhs_iter) const noexcept;

        /// @brief comparing list iterators on equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         there is no content for meta-elements like before_begin() and end()
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        /// @param[in] rhs_citer is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator==(const const_iterator rhs_citer) const noexcept;

        /// @brief dereferencing element content via iterator-position element
        /// @return reference to list element data
        T& operator*() const noexcept;

        /// @brief dereferencing element content via iterator-position element
        /// @return pointer to list element data
        T* operator->() const noexcept;

      private:
        /// @brief private construct for an iterator, the iterator is bundled to
        ///         an existing parent (object) of type forward_list,
        ///         an iterator is only constructed through calls to before_begin(), begin() or end()
        /// @param[in] parent is the forward_list the this iterator operates on
        /// @param[in] idx is the index of the list element (within allocated memory of parent list)
        explicit iterator(forward_list* parent, size_type idx) noexcept;

        using ListNodePointer = ListNode*;

        friend class forward_list<T, CAPACITY>;
        friend class const_iterator;
        forward_list<T, CAPACITY>* m_list;
        size_type m_iterListNodeIdx;

    }; // class iterator

    /// @brief nested const_iterator class, --> linked data element is 'const'
    class const_iterator
    {
      public:
        // provide the following public types for a std::iterator compatible iterator_category interface
        using iterator_category = std::forward_iterator_tag;
        using value_type = const T;
        using difference_type = void;
        using pointer = const T*;
        using reference = const T&;

      private:
        /// @brief private construct for an iterator, the iterator is bundled to
        ///         an existing parent (object) of type forward_list,
        ///         an iterator is only constructed through calls to before_begin(), begin() or end()
        /// @param[in] parent is the const forward_list the this iterator operates on
        /// @param[in] idx is the index of the list element (within allocated memory of parent list)
        explicit const_iterator(const forward_list* parent, size_type idx) noexcept;

      public:
        /// @brief construct a const_iterator from an (non-const_) iterator
        /// @param[in] iter is the iterator which will deliver list and index info for the const_iterator
        const_iterator(const iterator iter) noexcept;


        /// @brief prefix increment iterator, so it points to the next list element
        ///         when trying to increment beyond the end of the list,
        ///         iterator stays pointing at the end and a message is forwarded to the error_message
        ///         handler / cerr stream
        /// @return reference to this iterator object
        const_iterator& operator++() noexcept;


        /// @brief comparing list iterators on equality
        ///         the referenced list position is compared, not the content of the list element (T-typed)
        ///         there is no content for meta-elements like before_begin() and end()
        ///         only iterators of the same parent list can be compared; in case of misuse, terminate() is invoked
        /// @param[in] rhs_citer is the 2nd iterator to compare to
        /// @return list position for two iterators is the same (true) or different (false)
        bool operator==(const const_iterator rhs_citer) const noexcept;

        /// @brief dereferencing element content via iterator-position element
        /// @return reference to list element data
        const T& operator*() const noexcept;

        /// @brief dereferencing element content via iterator-position element
        /// @return pointer to const list data element
        const T* operator->() const noexcept;

      private:
        using CListNodePointer = const ListNode*;
        friend class forward_list<T, CAPACITY>;

        const forward_list<T, CAPACITY>* m_cList;
        size_type m_iterListNodeIdx;

    }; // class const_iterator

    /// @brief retrieve an interator before first element
    ///         only allowed for usage in erase_after, insert_after, emplace_after
    ///         Terminated when content is attemted to read (operator*, operator->)
    /// @return iterator to meta-element before first data element
    iterator before_begin() noexcept;

    /// @brief retrieve an const_iterator before first element
    ///         only allowed for usage in erase_after, insert_after, emplace_after
    /// @return iterator to meta-element before first data element
    const_iterator before_begin() const noexcept;

    /// @brief const_iterator an interator before first element
    ///         only allowed for usage in erase_after, insert_after, emplace_after
    /// @return iterator to meta-element before first data element
    const_iterator cbefore_begin() const noexcept;

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

    /// @brief list meta information
    /// @return list has been initialized with the following number of elements.
    size_type capacity() const noexcept;

    /// @brief list meta information
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

    /// @brief add element to the begin of the list
    /// @param[in] r_data reference to data element
    /// @return successful insertion (true), otherwise no element could be added to list (e.g. full -> false)
    bool push_front(const T& r_data) noexcept;

    /// @brief create element inplace at the begining of the list
    /// @param[in] ur_data universal reference perfectly forwarded to client class
    /// @return successful insertion (true), otherwise no element could be added to list (e.g. full -> false)
    bool push_front(T&& ur_data) noexcept;

    /// @brief remove the first element from the begining of the list
    ///         element destructor will be invoked
    /// @return successful removal (true), otherwise no element could be taken from list (e.g. empty -> false)
    bool pop_front() noexcept;

    /// @brief remove all elements from the list, list will be empty
    ///         element destructors will be invoked
    void clear() noexcept;

    /// @brief remove next element from linked iterator position
    ///         element destructors will be invoked
    ///         recursive calls to erase_after only delete each 2nd element
    /// @param[in] beforeToBeErasedIter iterator linking the element before the to-be-removed element
    /// @return an (non-const_) iterator to the element after the removed element,
    ///         returns end() element when reached end of list
    iterator erase_after(const_iterator beforeToBeErasedIter) noexcept;

    /// @brief construct element inplace at begining of list
    /// @param[in] args T-typed construction parameters (initializer list)
    template <typename... ConstructorArgs>
    /// @return successful insertion (true), otherwise no element could be added to list (e.g. full -> false)
    bool emplace_front(ConstructorArgs&&... args) noexcept;

    /// @brief construct element inplace at begining of list
    /// @param[in] args T-typed construction parameters (initializer list)
    /// @param[in] afterToBeEmplacedIter position in list to (construct)insert after
    /// @return iterator to the newly added element
    template <typename... ConstructorArgs>
    iterator emplace_after(const_iterator afterToBeEmplacedIter, ConstructorArgs&&... args) noexcept;

    /// @brief insert element after iterator position
    /// @param[in] citer iterator with the position to insert after
    /// @param[in] r_data reference to element to add
    /// @return iterator to the newly added element
    iterator insert_after(const_iterator citer, const T& r_data) noexcept;

    /// @brief construct element inplace at begining of list
    /// @param[in] citer iterator with the position to insert after
    /// @param[in] ur_data universal reference perfectly forwarded to client class
    /// @return iterator to the newly added element
    iterator insert_after(const_iterator citer, T&& ur_data) noexcept;

  private:
    struct ListNode
    {
        T data;
        size_type nextIdx;
    };

    void init() noexcept;
    ListNode* getNodePtrFromIdx(size_type) const noexcept;
    ListNode* getHeadOfFreeElementList() const noexcept;
    ListNode* getNodePtr() const noexcept;

    bool isValidIteratorIndex(size_type index) const noexcept;
    bool isValidElementIndex(size_type index) const noexcept;
    static void errorMessage(const char* f_source, const char* f_msg) noexcept;


    //***************************************
    //    members
    //***************************************

    // two extra slots in the list to handle the 'before_begin' element of each:(internal) freeList and usedList
    // the necessity for 'before_begin' elements stems from the way a forward_list removes elements at an arbitrary
    // position. Removing the front-most list element (aka begin()) requires an element pointing towards this position,
    // hence 'before_begin'. The before_begin index is the head of each free-slots or used-slots list respectively.
    static constexpr size_type INTERNAL_CAPACITY{size_type(CAPACITY) + 2U};
    static constexpr size_type BEFORE_BEGIN_USED_INDEX{0U};
    static constexpr size_type BEFORE_BEGIN_FREE_INDEX{1U};
    static constexpr size_type INVALID_INDEX{size_type(CAPACITY) + 2U};

    using element_t = uint8_t[sizeof(ListNode)];
    alignas(alignof(T)) element_t m_data[INTERNAL_CAPACITY];

    size_type m_size{0u};
}; // forward_list

namespace
{
/// @brief describe the non-euqal comparison via equal comparison
///         doing so allows for a more compact implementation of iterator and const_iterator class
///         anonymous namespace for only local visibility of declaration
/// @param[in] lhs (and rhs) any two classes for which a comparison operator (operator==) can be found,
/// @param[in] rhs any class for which a comparison operator (operator==) can be found
///          the inputs have different template types to allow for iterator and const_iterator combinations
/// @return the negative result of a comparison (operator==) of the inputs
template <typename U, typename V>
bool operator!=(U lhs, V rhs) noexcept;
} // namespace

} // namespace cxx
} // namespace iox

#include <iceoryx_utils/internal/cxx/forward_list.inl>

#endif // CXX_FORWARD_LIST_HPP_INCLUDED