// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

// TODO Decide on location, cxx does not fit since it has no STL counterpart
//      neither is it concurrent
//      It should be available publicly.
//      Decide on namespace.

#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/cxx/vector.hpp"
#include "iceoryx_hoofs/data_structures/typed_allocator.hpp"

namespace iox
{
template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength = 128>
class PrefixTree
{
  public:
    // using Key = std::string;
    // rename to Key
    using Key = cxx::string<MaxKeyLength>;

  private:
    // TODO: choose these values (requires space estimation for the structure)
    static constexpr uint32_t CAPACITY_LIMIT = 1 << 14;
    static constexpr uint32_t MAX_KEY_LENGTH_LIMIT = 1 << 8;

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

    static_assert(Capacity <= CAPACITY_LIMIT);
    static_assert(MaxKeyLength <= MAX_KEY_LENGTH_LIMIT);

    // order matters in those structs due to padding
    struct DataNode;
    struct DataNode
    {
        DataNode* next{nullptr};
        Value value; // todo: no default ctor requirement
    };

    // follow a de la briandais tree approach to save memory at cost of traversal time
    // note: the children could be managed by a hashmap if it was available (faster but more memory)
    // The data could be managed in another container as well, but this would be problematic with static memory again
    // We assume we usually have few data items per key, and hence a linked list is ok.
    // In the unique key case we have at most one data item, i.e. data is nullptr for non-keys
    // or the list contains exactly one item for a key in the prefix-tree.
    struct Node;
    struct Node
    {
        // 3 pointers on 64 Bit systems - 24 Bytes
        Node* child{nullptr};
        Node* sibling{nullptr};
        DataNode* data{nullptr};

        // actual content 1 Byte, but due to alignment the size of the struct is 32 Byte
        char letter;
    };

    using NodeAllocator = typed_allocator<Node, NUMBER_OF_ALLOCATABLE_NODES>;
    using DataNodeAllocator = typed_allocator<DataNode, Capacity>;

  private:
    Node* m_root{nullptr};
    uint32_t m_size{0U};

    DataNodeAllocator dataNodeAllocator;
    NodeAllocator nodeAllocator;

  public:
    PrefixTree() noexcept
    {
        m_root = allocateNode();
    }

    ~PrefixTree()
    {
        deleteRecursively(m_root);
    }

    // TODO: do we want to eliminate duplicate values?
    // do we want to distinguish error cases? I do not think we should, failing to insert is
    // due to resources (Capacity) being exhausted (if we allow duplicates)
    bool insert(const Key& key, const Value& value) noexcept
    {
        uint32_t length = key.size();
        if (length > MaxKeyLength)
        {
            return false;
        }

        auto data = allocateDataNode();
        if (!data)
        {
            return false;
        }
        data->value = value;

        const char* letters = key.c_str();
        uint32_t prefixLength;

        auto node = findPrefix(letters, length, prefixLength);

        // node will be at least the root node (if there is no prefix in the tree)

        if (prefixLength == length)
        {
            // the full string is already in the tree (either as a prefix of some other string or as a key itself)
            if (!node->data)
            {
                node->data = data;
            }
            else
            {
                // data already exists for this key, we add our value to the list
                // we could check for duplicates here, but would need to traverse the list
                // TODO: do we want to allow duplicates?
                data->next = node->data;
                node->data = data;
            }

            return true;
        }

        // only a strict prefix is already in the tree
        // we need to create the suffix path beyond node and then add our value to the final node

        const char* suffix = &letters[prefixLength];
        uint32_t suffix_len = length - prefixLength;

        auto suffixEndNode = addSuffix(node, suffix, suffix_len);

        if (!suffixEndNode)
        {
            // adding the suffix failed, clean up structure to restore state before insertion
            // if the node exists, we added it as the first node of the suffix path
            // but somehow failed adding the full suffix - remove these nodes since they are not needed
            // (otherwise we would not have tried to add the suffix)
            node = findInChildren(node, letters[prefixLength]);
            deleteRecursively(node);

            deallocateDataNode(data); // could not insert data, clean up prepared data node and return
            return false;
        }

        // created the suffix and can insert data at this node
        suffixEndNode->data = data;
        return true;
    }

    // TODO: do we want a value version? - findValues (can just copy the data out)
    // container can be changed or be a reference input (with fixed size)
    // can also return them by value ... inefficient
    cxx::vector<const Value*, Capacity> find(const Key& key) noexcept
    {
        auto node = findNode(key);

        cxx::vector<const Value*, Capacity> result;

        if (!node)
        {
            return result;
        }

        auto data = node->data;
        while (data)
        {
            result.push_back(&data->value);
            data = data->next;
        }
        return result;
    }

    bool remove(const Key& key) noexcept
    {
        auto node = findNode(key);
        if (!node || !node->data)
        {
            return false;
        }

        // delete all data at key
        deleteData(node);

        if (node->child)
        {
            // the node is needed since it has children (which lead to data otherwise they would not exist)
            return true;
        }

        // In theory we need to go up to the first node with data and only one child and delete all nodes on the way
        // However, we want to save the parent pointer and hence traverse again from the root instead
        // we could also optimize this to do it in the first downward path (where we are not sure if we find anything to
        // delete)

        removeNodes(key);

        return true;
    }

    bool remove(const Key& key, const Value& value) noexcept
    {
        auto node = findNode(key);
        if (!node || !node->data)
        {
            return false;
        }
        
        // if value does not exist at key, we cannot remove it
        auto removed = deleteValue(node, value);

        if (node->child)
        {
            // the node is needed since it has children (which lead to data otherwise they would not exist)
            return removed;
        }

        removeNodes(key);

        return removed;
    }

    uint32_t capacity() noexcept
    {
        return Capacity;
    }

    uint32_t size() noexcept
    {
        return m_size;
    }

    bool empty() noexcept
    {
        return size() == 0;
    }

  private:
    Node* allocateNode() noexcept
    {
        return nodeAllocator.create();
    }

    void deallocateNode(Node* node) noexcept
    {
        auto data = node->data;
        while (data)
        {
            auto next = data->next;
            deallocateDataNode(data);
            data = next;
        }
        nodeAllocator.destroy(node);
    }

    DataNode* allocateDataNode() noexcept
    {
        if (m_size < Capacity)
        {
            auto node = dataNodeAllocator.create();

            if (node)
            {
                ++m_size;
                return node;
            }
        }
        return nullptr;
    }

    void deallocateDataNode(DataNode* node) noexcept
    {
        --m_size;        
        dataNodeAllocator.destroy(node);
    }

    Node* findInChildren(Node* node, char letter) noexcept
    {
        node = node->child;
        while (node)
        {
            if (letter == node->letter)
            {
                return node;
            }
            node = node->sibling;
        }
        return nullptr;
    }

    // prefixLength will return the length of the existing maximum prefix in the tree
    Node* findPrefix(const char* letters, uint32_t length, uint32_t& prefixLength) noexcept
    {
        prefixLength = 0;
        auto node = m_root;
        while (node && prefixLength < length)
        {
            char letter = letters[prefixLength];
            auto matchingNode = findInChildren(node, letter);
            if (!matchingNode)
            {
                return node;
            }
            ++prefixLength;
            node = matchingNode;
        }

        return node;
    }

    Node* addSuffix(Node* node, const char* suffix, uint32_t length) noexcept
    {
        uint32_t i = 0;
        while (i < length)
        {
            char letter = suffix[i];

            // could be optimized as newly created nodes have no children (only the first)
            node = addChild(node, letter);
            if (!node)
            {
                return nullptr; // no nodes / memory left
            }
            ++i;
        }

        return node;
    }

    Node* addSibling(Node* node, char letter) noexcept
    {
        auto sibling = allocateNode();
        if (!sibling)
        {
            return nullptr;
        }
        sibling->letter = letter;
        sibling->sibling = node->sibling;
        node->sibling = sibling;
        return sibling;
    }

    Node* addChild(Node* node, char letter) noexcept
    {
        if (!node->child)
        {
            auto child = allocateNode();
            if (!child)
            {
                return nullptr;
            }
            child->letter = letter;
            node->child = child;

            return child;
        }

        return addSibling(node->child, letter);
    }

    Node* findNode(const Key& key) noexcept
    {
        const char* letters = key.c_str();
        uint32_t prefixLength;
        uint32_t length = key.size();
        auto node = findPrefix(letters, length, prefixLength);
        if (prefixLength < length)
        {
            return nullptr;
        }
        return node;
    }

    void deleteData(Node* node) noexcept
    {
        auto data = node->data;
        while (data)
        {
            auto next = data->next;
            deallocateDataNode(data);
            data = next;
        }
        node->data = nullptr;
    }

    bool deleteValue(Node* node, const Value& value)
    {
        auto data = node->data;
        if (!data)
        {
            return false;
        }

        bool deleted = false;

        // assume that value can occur multiple times

        // delete front values if equal to value
        while (data && data->value == value)
        {
            node->data = data->next;
            deallocateDataNode(data);
            deleted = true;
            data = node->data;
        }

        node->data = data;
        if (!data)
        {
            // no values left
            return deleted;
        }

        // front is not value
        auto prev = data;
        data = prev->next;

        while (data)
        {
            auto next = data->next;
            if (data->value == value)
            {
                // unlink data and delete it
                deallocateDataNode(data);
                deleted = true;
                prev->next = next;
            }
            else
            {
                prev = data;
            }
            data = next;
        }
        return deleted;
    }

    void deleteRecursively(Node* node) noexcept
    {
        if (node)
        {
            auto child = node->child;
            while (child)
            {
                auto sibling = child->sibling;
                deleteRecursively(child);
                child = sibling;
            }
            deallocateNode(node);
        }
    }

    Node* findClosestNodeToRootToDelete(const char* letters, uint32_t length, Node** parent) noexcept
    {
        uint32_t prefix = 0;
        auto node = m_root;
        Node* deleteableNode{nullptr};

        while (prefix < length)
        {
            char letter = letters[prefix];
            auto next = findInChildren(node, letter);

            // Note: we could do this in the first downward pass of a search implementation for remove (but not the one
            // for find, there it is not needed)
            if (next->data)
            {
                deleteableNode = nullptr;
            }
            else
            {
                if (next->child && next->child->sibling)
                {
                    deleteableNode = nullptr;
                }
                else
                {
                    if (!deleteableNode)
                    {
                        *parent = node;
                        deleteableNode = next;
                    }
                }
            }
            ++prefix;
            node = next;
        }

        return deleteableNode;
    }

    void removeNodes(const Key& key)
    {
        uint32_t length = key.size();
        const char* letters = key.c_str();
        Node* parent;

        // this is the node closest to the root from which we can safely delete the path downwards
        // since it will not be used to store data
        // (i.e. all higher nodes are on paths to actual data and cannot be deleted)
        auto node = findClosestNodeToRootToDelete(letters, length - 1, &parent);

        if (node)
        {
            // remove from child list of parent
            auto child = parent->child;
            if (child == node)
            {
                parent->child = child->sibling;
            }
            else
            {
                while (child)
                {
                    if (child->sibling == node)
                    {
                        child->sibling = node->sibling;
                        break;
                    }
                    child = child->sibling;
                }
            }
            deleteRecursively(node);
        }
    }
};

} // namespace iox


#endif // IOX_HOOFS_DATA_STRUCTURES_PREFIX_TREE_HPP