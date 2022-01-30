// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/data_structures/prefix_tree.hpp"

namespace iox
{
namespace cxx
{
template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
PrefixTree<Value, Capacity, MaxKeyLength>::PrefixTree() noexcept
{
    m_root = allocateNode();
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
PrefixTree<Value, Capacity, MaxKeyLength>::~PrefixTree() noexcept
{
    deleteTree(m_root);
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
bool PrefixTree<Value, Capacity, MaxKeyLength>::insert(const Key& key, const Value& value) noexcept
{
    uint32_t length = (uint32_t)key.size(); // todo: fix conversion
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
        if (node->data == nullptr)
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
        // if node exists, we added it as the first node of the suffix path
        // but somehow failed adding the full suffix - remove these nodes since they are not needed
        // (otherwise we would not have tried to add the suffix)
        node = findInChildren(node, letters[prefixLength]);
        deleteTree(node);

        deallocateDataNode(data); // could not insert data, clean up prepared data node and return
        return false;
    }

    // created the suffix and can insert data at this node
    suffixEndNode->data = data;

    return true;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
cxx::vector<Value*, Capacity> PrefixTree<Value, Capacity, MaxKeyLength>::find(const Key& key) const noexcept
{
    auto node = findNode(key);

    cxx::vector<Value*, Capacity> result;

    if (!node)
    {
        return result;
    }

    getValuesFromNode(node, result);
    return result;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
cxx::vector<Value*, Capacity> PrefixTree<Value, Capacity, MaxKeyLength>::findPrefix(const Key& prefix) const noexcept
{
    auto node = findNode(prefix);

    cxx::vector<Value*, Capacity> result;

    if (!node)
    {
        return result;
    }

    getValuesFromSubTree(node, result);

    return result;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
bool PrefixTree<Value, Capacity, MaxKeyLength>::remove(const Key& key) noexcept
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

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
bool PrefixTree<Value, Capacity, MaxKeyLength>::remove(const Key& key, const Value& value) noexcept
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

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
cxx::vector<typename PrefixTree<Value, Capacity, MaxKeyLength>::Key, Capacity>
PrefixTree<Value, Capacity, MaxKeyLength>::keys() noexcept
{
    cxx::vector<Key, Capacity> result;
    char currentString[MaxKeyLength + 1];
    currentString[MaxKeyLength] = '\0';

    // root must be handled differently, it represents the empty string key
    if (m_root->data)
    {
        result.emplace_back("");
    }

    Node* child = m_root->child;
    while (child)
    {
        getKeys(child, 1, currentString, result);
        child = child->sibling;
    }

    return result;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
cxx::vector<Value*, Capacity> PrefixTree<Value, Capacity, MaxKeyLength>::values() const noexcept
{
    Key emptyPrefix;
    return findPrefix(emptyPrefix);
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
cxx::vector<std::pair<typename PrefixTree<Value, Capacity, MaxKeyLength>::Key, Value*>, Capacity>
PrefixTree<Value, Capacity, MaxKeyLength>::keyValuePairs() noexcept
{
    cxx::vector<std::pair<Key, Value*>, Capacity> result;
    char currentString[MaxKeyLength + 1];
    currentString[MaxKeyLength] = '\0';

    // root must be handled differently, it represents the empty string key
    if (m_root->data)
    {
        Key key(iox::cxx::TruncateToCapacity, "");
        auto data = m_root->data;
        do
        {
            auto pair = std::make_pair(key, &data->value);
            result.push_back(pair);
            data = data->next;
        } while (data);
    }

    Node* child = m_root->child;
    while (child)
    {
        getPairs(child, 1, currentString, result);
        child = child->sibling;
    }

    return result;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
uint32_t PrefixTree<Value, Capacity, MaxKeyLength>::capacity() noexcept
{
    return Capacity;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
uint32_t PrefixTree<Value, Capacity, MaxKeyLength>::size() noexcept
{
    return m_size;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
bool PrefixTree<Value, Capacity, MaxKeyLength>::empty() noexcept
{
    return size() == 0;
}

//****************************************************************
// private functions
//****************************************************************

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::Node*
PrefixTree<Value, Capacity, MaxKeyLength>::allocateNode() noexcept
{
    auto node = nodeAllocator.create();
    return node;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::deallocateNode(Node* node) noexcept
{
    // we cannot use auto here since we want a raw pointer (local computation, efficiency)
    // node->data may be a smart pointer type (e.g. relocatable_ptr)
    DataNode* data = node->data;
    while (data)
    {
        DataNode* next = data->next;
        deallocateDataNode(data);
        data = next;
    }
    nodeAllocator.destroy(node);
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::DataNode*
PrefixTree<Value, Capacity, MaxKeyLength>::allocateDataNode() noexcept
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

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::deallocateDataNode(DataNode* node) noexcept
{
    --m_size;
    dataNodeAllocator.destroy(node);
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::Node*
PrefixTree<Value, Capacity, MaxKeyLength>::findInChildren(Node* node, char letter) const noexcept
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

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::Node* PrefixTree<Value, Capacity, MaxKeyLength>::findPrefix(
    const char* letters, uint32_t length, uint32_t& prefixLength) const noexcept
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

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::Node*
PrefixTree<Value, Capacity, MaxKeyLength>::addSuffix(Node* node, const char* suffix, uint32_t length) noexcept
{
    uint32_t i = 0;
    while (i < length)
    {
        char letter = suffix[i];

        // could be optimized as newly created nodes have no children (only the first)
        node = addChild(node, letter);
        if (!node)
        {
            return nullptr; // no nodes (memory) left
        }
        ++i;
    }

    return node;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::Node*
PrefixTree<Value, Capacity, MaxKeyLength>::addSibling(Node* node, char letter) noexcept
{
    auto sibling = allocateNode();
    if (!sibling)
    {
        return nullptr;
    }
    sibling->letter = letter;

    // add the sibling sorted (allows lexicographic enumeration)
    while (node->sibling && letter > node->letter)
    {
        node = node->sibling;
    }

    sibling->sibling = node->sibling;
    node->sibling = sibling;
    return sibling;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::Node*
PrefixTree<Value, Capacity, MaxKeyLength>::addChild(Node* node, char letter) noexcept
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

    // there exists a first child but it has a greater letter value,
    // add the letter as new first child (i.e. sorted)

    if (letter < node->child->letter)
    {
        auto child = allocateNode();
        if (!child)
        {
            return nullptr;
        }
        child->letter = letter;
        child->sibling = node->child;
        node->child = child;

        return child;
    }

    // there already exist children and the first one has a smaller letter,
    // add the new letter sorted to the children

    return addSibling(node->child, letter);
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::Node*
PrefixTree<Value, Capacity, MaxKeyLength>::findNode(const Key& key) const noexcept
{
    const char* letters = key.c_str();
    uint32_t prefixLength;
    uint32_t length = (uint32_t)key.size(); // todo: fix conversion
    auto node = findPrefix(letters, length, prefixLength);
    if (prefixLength < length)
    {
        return nullptr;
    }
    return node;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::deleteData(Node* node) noexcept
{
    DataNode* data = node->data;
    while (data)
    {
        DataNode* next = data->next;
        deallocateDataNode(data);
        data = next;
    }
    node->data = nullptr;
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
bool PrefixTree<Value, Capacity, MaxKeyLength>::deleteValue(Node* node, const Value& value)
{
    DataNode* data = node->data;
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
    DataNode* prev = data;
    data = prev->next;

    while (data)
    {
        DataNode* next = data->next;
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

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::deleteTree(Node* node) noexcept
{
    if (node)
    {
        Node* child = node->child;
        while (child)
        {
            Node* sibling = child->sibling;
            deleteTree(child);
            child = sibling;
        }
        deallocateNode(node);
    }
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
typename PrefixTree<Value, Capacity, MaxKeyLength>::Node*
PrefixTree<Value, Capacity, MaxKeyLength>::findClosestNodeToRootToDelete(const char* letters,
                                                                         uint32_t length,
                                                                         Node** parent) noexcept
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

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::removeNodes(const Key& key) noexcept
{
    uint32_t length = (uint32_t)key.size(); // todo fix cast
    if (length == 0)
    {
        // we are at root (empty string),
        // nothing to delete on the path from root to the node storing the empty string (= root)
        return;
    }

    const char* letters = key.c_str();
    Node* parent{nullptr};

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
        deleteTree(node);
    }
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::getValuesFromNode(Node* node,
                                                                  cxx::vector<Value*, Capacity>& result) const noexcept
{
    auto data = node->data;
    while (data)
    {
        result.push_back(&data->value);
        data = data->next;
    }
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::getValuesFromSubTree(
    Node* node, cxx::vector<Value*, Capacity>& result) const noexcept
{
    getValuesFromNode(node, result);

    Node* child = node->child;
    while (child)
    {
        getValuesFromSubTree(child, result);
        child = child->sibling;
    }
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::getKeys(Node* node,
                                                        uint32_t depth,
                                                        char* currentString,
                                                        cxx::vector<Key, Capacity>& result) noexcept
{
    if (!node)
    {
        return;
    }

    currentString[depth - 1] = node->letter;

    if (node->data)
    {
        currentString[depth] = '\0';
        Key key(iox::cxx::TruncateToCapacity, currentString);
        result.push_back(key);
    }

    Node* child = node->child;
    while (child)
    {
        getKeys(child, depth + 1, currentString, result);
        child = child->sibling;
    }
}

template <typename Value, uint32_t Capacity, uint32_t MaxKeyLength>
void PrefixTree<Value, Capacity, MaxKeyLength>::getPairs(Node* node,
                                                         uint32_t depth,
                                                         char* currentString,
                                                         cxx::vector<std::pair<Key, Value*>, Capacity>& result) noexcept
{
    if (!node)
    {
        return;
    }

    currentString[depth - 1] = node->letter;

    if (node->data)
    {
        currentString[depth] = '\0';
        Key key(iox::cxx::TruncateToCapacity, currentString);

        auto data = node->data;
        do
        {
            auto pair = std::make_pair(key, &data->value);
            result.push_back(pair);
            data = data->next;
        } while (data);
    }

    Node* child = node->child;
    while (child)
    {
        getPairs(child, depth + 1, currentString, result);
        child = child->sibling;
    }
}

} // namespace cxx

} // namespace iox
