// Copyright (c) 2021-2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_DATA_STRUCTURES_PREFIX_TREE_HPP
#define IOX_HOOFS_DATA_STRUCTURES_PREFIX_TREE_HPP

#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/data_structures/typed_allocator.hpp"
#include "iceoryx_hoofs/internal/relocatable_pointer/relocatable_ptr.hpp"

namespace iox
{
namespace cxx
{
/// @brief A prefix tree to store key-value pairs for fast string prefix search.
///        Keys are always strings (cxx::string) of configurable maximum length.
///        Supports fast insertion, removal and search of key-value pairs.
///        The value type can be configured by the user.
///        Similar to multi-maps, each key can have multiple corresponding values.
///        Insertion and removal have O(key length) complexity.
///        Search has the same complexity if we ignore the cost of constructing the
///        search result set, which can contain at most Capacity values (worst-case).
///        The search interface avoids copying for efficiency and allows direct
///        access of the found values (similar to a cxx::vector).
/// @tparam Value type of values to be stored
/// @tparam Capacity maximum number of values to be stored.
/// @tparam MaxKeyLength maximum length of the key strings
/// @note relocatable, i.e. logical state can be copied using memcpy
template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength = 128>
class PrefixTree
{
  public:
    // We only support specific string keys for now.
    // NB: In general any sequence of letters with an order would work
    //     but currently we only support char letters.
    using Key = cxx::string<MaxKeyLength>;

  public:
    /// @brief Construct an empty prefix tree.
    PrefixTree() noexcept;

    ~PrefixTree() noexcept;

    /// @todo implement copy later if needed, default may suffice since it is relocatable
    ///       move is not intended (would be a copy)
    PrefixTree(const PrefixTree& other) = delete;
    PrefixTree(PrefixTree&& other) = delete;
    PrefixTree& operator=(const PrefixTree&) = delete;
    PrefixTree& operator=(PrefixTree&&) = delete;

    /// @brief Insert a key and a correspoding value into the tree.
    /// @param key key to be inserted
    /// @param value value to be inserted
    /// @return true if insertion was successful, false otherwise (capacity exceeded)
    /// @note allows multiple values associated with a key, including duplicate values
    bool insert(const Key& key, const Value& value) noexcept;

    /// @brief Remove a key and all its associated values from the tree.
    /// @param key key to be removed
    /// @return true if was removed, false otherwise (key did not exist)
    bool remove(const Key& key) noexcept;

    /// @brief Remove a key and a corresponding value from the tree.
    /// @param key key to be removed
    /// @param value value to be removed
    /// @return true if (key, value) was removed, false otherwise
    ///         (key with this value did not exist)
    bool remove(const Key& key, const Value& value) noexcept;

    /// @note: pointer interface (Value*) is required for efficiency
    ///        and to access values for modification

    /// @todo value versions of find and similar functions? - copies values out
    /// @todo callback interface of find and similar functions?
    ///       pass a lambda to be invoked on matches to avoid returning large capacity
    ///       containers

    /// @brief Find all values corresponding to a given key.
    /// @param key key whose values we want to find
    /// @return pointers to values matching the key
    /// @note returned pointers allow modification of corresponding values in the tree
    cxx::vector<Value*, Capacity> find(const Key& key) const noexcept;

    /// @brief Find all values corresponding to keys with a given prefix.
    /// @param prefix prefix of keys whose values we want to find
    /// @return pointers to values matching the key
    /// @note returned pointers allow modification of corresponding values in the tree
    cxx::vector<Value*, Capacity> findPrefix(const Key& prefix) const noexcept;

    /// @brief Get all key strings currently in the tree.
    /// @return keys currently in the tree
    /// @note returned keys are copies since keys are not contiguously stored internally
    cxx::vector<Key, Capacity> keys() noexcept;

    /// @brief Get all values currently in the tree.
    /// @return pointers to values currently in the tree
    /// @note returned pointers allow modification of corresponding values in the tree
    cxx::vector<Value*, Capacity> values() const noexcept;

    /// @brief Get all key value pairs currently in the tree.
    /// @return key value pointer pairs corresponding to values currently in the tree
    /// @note returned pointers allow modification of corresponding values in the tree
    cxx::vector<std::pair<Key, Value*>, Capacity> keyValuePairs() noexcept;

    /// @brief Get the capacity of the tree
    ///        (maximum number of key-value pairs that can be stored)
    /// @return capacity
    uint32_t capacity() noexcept;

    /// @brief Get the capacity of the tree
    ///        (maximum number of key-value pairs that can be stored)
    /// @return capacity
    uint32_t size() noexcept;

    bool empty() noexcept;

    /// @note The interface is sufficient for the general use case
    ///       but may be extended in the future with more convenience functions
    ///       (e.g. lambdas, iterators, ...).

  private:
    // Limit of the maximum number of entries (key-value pairs) that can be configured
    // by the Capacity template parameter.
    static constexpr uint32_t CAPACITY_LIMIT = 1 << 20;

    // Maximum size of keys that can be configured by the MaxKeyLength template parameter.
    static constexpr uint32_t MAX_KEY_LENGTH_LIMIT = 512;

    // Number of internal nodes that can be allocated by the tree (for the search structure).
    // These are enough to store Capacity data elements in the worst case (no shared prefixes),
    // but this costs alot of memory (usually there are shared prefixes).
    // This is also responsible for a large part of the size of the data structure itself.
    // We can improve this estimate somewhat by noticing that with a limited alphabet we
    // will share nodes close to the root (pigeon hole principle).
    // However, this will not help much.
    // On average only a small fraction of these nodes will be needed.
    // We may use this and not prepare for the worst case but make this number configruable in a reasonable way.
    static constexpr uint64_t NUMBER_OF_ALLOCATABLE_NODES = Capacity * MaxKeyLength;

    static_assert(Capacity <= CAPACITY_LIMIT, "Capacity exceeds limit");
    static_assert(MaxKeyLength <= MAX_KEY_LENGTH_LIMIT, "MaxKeyLength exceeds limit");

    // NB: This can be replaced with a raw pointer abstraction for added efficiency,
    //     but the resulting structure will not be relocatable.
    //     Could be exposed as template argument.
    //     Raw pointer abstraction:
    //     template <typename T>
    //     using raw_ptr = T*;
    template <typename T>
    using ptr_t = rp::relocatable_ptr<T>;

    // The advantage of using relocatble_ptr is that we can easily exchange them with raw pointers
    // when we do not need relocatable structures but can develop structures (almost) as with raw pointers.
    // This is in contrast to a much different implementation based purely on internal indices
    // The price is some small overhead in pointer dereferencing, but this is comparable
    // to the index indirection required that would be required as well when using indices internally.

    // NB: We could compress multiple nodes to e.g. represent not just one letter but multiple letters,
    //     but only in certain cases and during insertion uncompressing/splitting may be required
    // This is left as a space optimization in the future, but with static memory we cannot really as we
    // as we would need dynamic strings for the letters (one or more letters, compressed in a node).

    // Stores user data, i.e. the values.
    // Since each key can have multiple values, we store those values in a list associated with the key.
    struct DataNode;

    // the stored pointer type (for passing in local functions raw pointers are sufficient and more efficient)
    using data_node_ptr_t = ptr_t<DataNode>;
    struct DataNode
    {
        data_node_ptr_t next{nullptr}; // needed since we may have more than one entry per key (as in a multimap)
        Value value;                   // currently requires Value to be default-constructible
    };

    // This follows a "de la briandais tree" approach to save memory at cost of traversal time
    // Note: the children could be managed by a hashmap if it was available (faster but more memory)
    // The data could be managed in another container as well, but this would be problematic with static memory again
    // We assume we usually have few data items per key, and hence a linked list is ok.
    // In the unique key case we have at most one data item, i.e. data is nullptr for non-keys
    // or the list contains exactly one item for a key in the prefix-tree.

    // Stores meta-data related to the keys and establishes the search structure.
    // Each node stores only one character (letter) of some key and keys sharing the same
    // prefix will share their nodes.
    struct Node;

    // the stored pointer type
    using node_ptr_t = ptr_t<Node>;

    struct Node
    {
        // 3 pointers on 64 Bit systems - 24 Bytes
        node_ptr_t child{nullptr};     // needed to find the first child one level below
        node_ptr_t sibling{nullptr};   // needed to find its sibling on the same level
        data_node_ptr_t data{nullptr}; // needed to locate corresponding data

        // actual content 1 Byte, but due to alignment the size of the struct is 32 Byte
        char letter; // the letter as part of a key in the path from the root to a leaf
                     // the key may end at an intermediate node (i.e. no leaf)
    };

    // We use internal relocatable allocators to create the two kinds of nodes as needed.
    // The allocators are owned by the prefix tree itself (for easy resource and ownership management),
    // but in the future it is conceivable to just use (relocatable) pointers to
    // suitable external allocators.
    // The allocators can allocate enough nodes to support the specified capacity.

    using NodeAllocator = iox::cxx::TypedAllocator<Node, NUMBER_OF_ALLOCATABLE_NODES>;
    using DataNodeAllocator = iox::cxx::TypedAllocator<DataNode, Capacity>;

    DataNodeAllocator m_dataNodeAllocator;
    NodeAllocator m_nodeAllocator;

    // We need the root node as a starting point for insertion, removal and search operations.
    node_ptr_t m_root{nullptr};
    uint32_t m_size{0U};

  private:
    // internal functions operate on pointers for efficiency
    // relocatable pointers are not required for local computation

    Node* allocateNode() noexcept;

    void deallocateNode(Node* node) noexcept;

    DataNode* allocateDataNode() noexcept;

    void deallocateDataNode(DataNode* node) noexcept;

    Node* findInChildren(Node* node, char letter) const noexcept;

    // prefixLength will contain the length of the existing maximum prefix of letters
    // already in the tree (could be all letters)
    // This information is needed to locate existing entries efficiently.
    Node* findPrefix(const char* letters, uint32_t length, uint32_t& prefixLength) const noexcept;

    Node* addSuffix(Node* node, const char* suffix, uint32_t length) noexcept;

    Node* addSibling(Node* node, char letter) noexcept;

    Node* addChild(Node* node, char letter) noexcept;

    Node* findNode(const Key& key) const noexcept;

    void deleteData(Node* node) noexcept;

    bool deleteValue(Node* node, const Value& value);

    void deleteTree(Node* node) noexcept;

    Node* findClosestNodeToRootToDelete(const char* letters, uint32_t length, Node** parent) noexcept;

    void removeNodes(const Key& key) noexcept;

    void getValuesFromNode(Node* node, cxx::vector<Value*, Capacity>& result) const noexcept;

    void getValuesFromSubTree(Node* node, cxx::vector<Value*, Capacity>& result) const noexcept;

    void getKeys(Node* node, uint32_t depth, char* currentString, cxx::vector<Key, Capacity>& result) noexcept;

    void getPairs(Node* node,
                  uint32_t depth,
                  char* currentString,
                  cxx::vector<std::pair<Key, Value*>, Capacity>& result) noexcept;
};


} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/data_structures/prefix_tree.inl"


#endif // IOX_HOOFS_DATA_STRUCTURES_PREFIX_TREE_HPP