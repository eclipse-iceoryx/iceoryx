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

namespace iox
{
template <typename T, uint32_t Capacity, uint32_t MaxKeyLength = 128>
class PrefixTree
{
    // using string_t = std::string;
    using string_t = cxx::string<MaxKeyLength>;

    // TODO: chose these values (requires space estimation for the structure)
    static constexpr uint32_t CAPACITY_LIMIT = 1 << 14;
    static constexpr uint32_t MAX_KEY_LENGTH_LIMIT = 1 << 8;
    static_assert(Capacity <= CAPACITY_LIMIT);
    static_assert(MaxKeyLength <= MAX_KEY_LENGTH_LIMIT);

    // TODO: allocation must be index based without dynamic memory.
    // This will be done after the functionality is complete.
    // 1) change to static allocator - no dynamic memory
    // 2) change to index based structures - relocatable

    // order matters in those structs due to padding
    struct DataNode;
    struct DataNode
    {
        DataNode* next{nullptr};
        T value; // todo: no default ctor requirement
    };

    // follow a de la briandais tree approach to save memory at cost of traversal time
    struct Node;
    struct Node
    {
        Node* child{nullptr};
        Node* sibling{nullptr};
        DataNode* data{nullptr};

        char letter;
    };

  private:
    Node* m_root{nullptr};
    uint32_t m_size{0U};

  public:
    PrefixTree()
    {
        m_root = allocateNode();
    }

    ~PrefixTree()
    {
        deleteRecursively(m_root);
    }

    const T* insert(const string_t& key, const T& value)
    {
        uint32_t length = key.size();
        if (length > MaxKeyLength)
        {
            return nullptr;
        }

        auto data = allocateDataNode();
        if (!data)
        {
            return nullptr;
        }
        data->value = value;

        const char* letters = key.c_str();
        uint32_t prefixLength;

        auto node = findPrefix(letters, length, prefixLength);

        // will be at least the root node (if there is no prefix in the tree)

        if (prefixLength == length)
        {
            // entry data already exists, we add our value to the list
            if (!node->data)
            {
                node->data = data;
            }
            else
            {
                data->next = node->data;
                node->data = data;
            }

            return &data->value;
        }

        // we need to create the suffix path beyond node and then add our value to the final node

        const char* suffix = &letters[prefixLength];
        uint32_t suffix_len = length - prefixLength;

        node = addSuffix(node, suffix, suffix_len);

        if (!node)
        {
            return nullptr; // no memory - should not happen in the pool version later? TODO: cleanup unused
                            // intermediate nodes
        }

        node->data = data;

        return &data->value;
    }

    // container can be changed or be a reference input (with fixed size)
    // can also return them by value ... inefficient
    cxx::vector<const T*, Capacity> find(const string_t& key)
    {
        auto node = findNode(key);

        cxx::vector<const T*, Capacity> result;

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

    bool remove(const string_t& key)
    {
        auto node = findNode(key);
        if (!node)
        {
            return false;
        }

        deleteData(node); // delete all data at this node

        if (node->child)
        {
            // the node is needed since it has children (which lead to data otherwise they would not exist)
            return true;
        }

        // in theory we need to go up to the first node with data and only one child and delete all nodes on the way
        // however, we want to save the parent pointer and traverse again from the root instead

        uint32_t length = key.size();
        uint32_t prefix_len;
        const char* letters = key.c_str();
        Node* parent;

        // this is the node closest to the root from which we can sefaly delete the path downwards
        // since it will not be used to store data
        // (i.e. all higher nodes are on paths to actual data and cannot be deleted)
        node = findNodeClosestNodeToRootToDelete(letters, length - 1, &parent);

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

        return true;
    }

    uint32_t size()
    {
        return m_size;
    }

    bool empty()
    {
        return size() == 0;
    }

  private:
    Node* allocateNode()
    {
        return new Node; // later allocate from a preallocated pool belonging to the object itself
    }

    void deallocateNode(Node* node)
    {
        auto data = node->data;
        while (data)
        {
            auto next = data->next;
            deallocateDataNode(data);
            data = next;
        }
        delete node;
    }

    DataNode* allocateDataNode()
    {
        if (m_size < Capacity)
        {
            auto node = new DataNode; // later from a pool
            if (node)
            {
                ++m_size;
            }
            return node;
        }
        return nullptr;
    }

    void deallocateDataNode(DataNode* node)
    {
        --m_size;
        delete node;
    }

    Node* findInChildren(Node* node, char letter)
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
    Node* findPrefix(const char* letters, uint32_t length, uint32_t& prefixLength)
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

    Node* addSuffix(Node* node, const char* suffix, uint32_t length)
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

    Node* addSibling(Node* node, char letter)
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

    Node* addChild(Node* node, char letter)
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

    Node* findNode(const string_t& key)
    {
        const char* letters = key.c_str();
        uint32_t prefixLength;
        uint32_t length = key.length();
        auto node = findPrefix(letters, length, prefixLength);
        if (prefixLength < length)
        {
            return nullptr;
        }
        return node;
    }

    void deleteData(Node* node)
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

    void deleteRecursively(Node* node)
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

    Node* findNodeClosestNodeToRootToDelete(const char* letters, uint32_t length, Node** parent)
    {
        uint32_t prefix = 0;
        auto node = m_root;
        Node* deleteableNode{nullptr};

        while (prefix < length)
        {
            char letter = letters[prefix];
            auto next = findInChildren(node, letter);

            // TODO: improve logic (or refactor to do this in the first downward pass of the search already)
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
};

} // namespace iox


#endif // IOX_HOOFS_DATA_STRUCTURES_PREFIX_TREE_HPP