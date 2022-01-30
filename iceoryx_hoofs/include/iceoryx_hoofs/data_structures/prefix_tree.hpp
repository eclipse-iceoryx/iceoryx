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

// TODO: Decide on location, cxx does not fit since it has no STL counterpart
//      neither is it concurrent
//      It should be available publicly.
//      Decide on namespace.
//      Maybe add a folder relocatable structures ? (but list and vector presumably are also relocatable)

#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/data_structures/typed_allocator.hpp"
#include "iceoryx_hoofs/internal/relocatable_pointer/relocatable_ptr.hpp"

// TODO: discuss location
namespace iox
{
namespace cxx
{
template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength = 128>
class PrefixTree
{
  public:
    using Key = cxx::string<MaxKeyLength>;

  private:
    // TODO: choose these values (requires space estimation for the structure)
    static constexpr uint32_t CAPACITY_LIMIT = 1 << 14;
    static constexpr uint32_t MAX_KEY_LENGTH_LIMIT = 512;

    // Number of internal nodes that can be allocated by the tree (for the serach structure)
    // these are enough to store Capacity data elements in the worst case (no shared prefixes),
    // but this will waste a lot of memory.
    // This is also responsible for a large part of the size of the data structure itself.
    // We can improve this estimate somewhat by noticing that with a limited alphabet we
    // will share nodes close to the root (pigeon hole principle).
    // However, this will not help much.
    // On average only a small fraction of these nodes will be needed.
    // We may use this and not prepare for the worst case but make this number configruable in a reasonable way.
    static constexpr uint64_t NUMBER_OF_ALLOCATABLE_NODES = Capacity * MaxKeyLength;

    static_assert(Capacity <= CAPACITY_LIMIT, "Capacity exceeds limit");
    static_assert(MaxKeyLength <= MAX_KEY_LENGTH_LIMIT, "MaxKeyLength exceeds limit");

    template <typename T>
    using raw_ptr = T*;

    // used for storage but not local calculations (for this raw pointers are sufficient)
    // could be made available in a template argument and set raw_ptr by default
    template <typename T>
    // using ptr_t = raw_ptr<T>;
    using ptr_t = rp::relocatable_ptr<T>;

    // The advantage of using relocatble_ptrs is that we can switch hem of when we do not need
    // the feature but can develop structures (almost) as with normal pointers
    // instead of a much different implementation based purely on internal indices
    // The price is some small overhead in the pointer lookup, but this is comparable
    // to the index indirection required that would be required as well:
    // compute the pointer from index and access it, instead of directly accessing the pointer.

    // order matters in those structs due to padding
    struct DataNode;
    using data_node_ptr_t = ptr_t<DataNode>;
    struct DataNode
    {
        data_node_ptr_t next{nullptr}; // needed since we may have more than one entry per key (as in a multimap)
        Value value;                   // todo: no default ctor requirement on Value - optional
    };

    // follow a de la briandais tree approach to save memory at cost of traversal time
    // note: the children could be managed by a hashmap if it was available (faster but more memory)
    // The data could be managed in another container as well, but this would be problematic with static memory again
    // We assume we usually have few data items per key, and hence a linked list is ok.
    // In the unique key case we have at most one data item, i.e. data is nullptr for non-keys
    // or the list contains exactly one item for a key in the prefix-tree.
    struct Node;
    using node_ptr_t = ptr_t<Node>;


    struct Node
    {
        // 3 pointers on 64 Bit systems - 24 Bytes
        node_ptr_t child{nullptr};     // needed to find the first child one level below
        node_ptr_t sibling{nullptr};   // needed to find its sibling on the same level
        data_node_ptr_t data{nullptr}; // needed to locate corresponding data

        // actual content 1 Byte, but due to alignment the size of the struct is 32 Byte
        char letter; // the letter as part of a key in te path downward
    };

    // Remark: We can compress multiple Nodes to e.g. represent not just one letter but multiple letters,
    //         but only in certain cases and during insertion uncompressing/splitting may be required
    // This is left as a space optimization in the future, but with static memory we cannot really benefit
    // since we need to be prepared for single letter nodes anyway.

    using NodeAllocator = iox::cxx::TypedAllocator<Node, NUMBER_OF_ALLOCATABLE_NODES>;
    using DataNodeAllocator = iox::cxx::TypedAllocator<DataNode, Capacity>;

  private:
    node_ptr_t m_root{nullptr};
    uint32_t m_size{0U};

    DataNodeAllocator dataNodeAllocator;
    NodeAllocator nodeAllocator;

  public:
    PrefixTree() noexcept;

    ~PrefixTree() noexcept;

    // TODO: implement later - must perform a logical copy but in the relocatable case a memcpy
    // suffices (how do we optimize this?)
    // Care must be taken, the allocators must be relocatable as well which they are currently not
    // (to achieve this, they need to be index based internally)
    PrefixTree(const PrefixTree& other) = delete;
    PrefixTree(PrefixTree&& other) = delete;

    // TODO: finalize API
    // TODO: discuss error handling

    // TODO: do we want to eliminate duplicate values?
    inline bool insert(const Key& key, const Value& value) noexcept;

    // Value* interface is intentional for efficiency and access of values for modification

    // TODO: do we want a value version? - findValues (can just copy the data out)
    // container can be changed or be a reference input (with fixed size)
    // can also return them by value ... inefficient
    cxx::vector<Value*, Capacity> find(const Key& key) const noexcept;

    cxx::vector<Value*, Capacity> findPrefix(const Key& prefix) const noexcept;

    cxx::vector<Value*, Capacity> values() const noexcept;

    cxx::vector<std::pair<Key, Value*>, Capacity> keyValuePairs() noexcept;

    bool remove(const Key& key) noexcept;

    bool remove(const Key& key, const Value& value) noexcept;

    // TODO: key value pairs?
    cxx::vector<Key, Capacity> keys() noexcept;

    uint32_t capacity() noexcept;

    uint32_t size() noexcept;

    bool empty() noexcept;

  private:
    Node* allocateNode() noexcept;

    void deallocateNode(Node* node) noexcept;

    DataNode* allocateDataNode() noexcept;

    void deallocateDataNode(DataNode* node) noexcept;

    Node* findInChildren(Node* node, char letter) const noexcept;

    // prefixLength will return the length of the existing maximum prefix in the tree
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