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

#pragma once

#include <algorithm>

namespace iox
{
namespace cxx
{
namespace set
{
/// Adds entry to a container with set semantics. The entry type must provide operator==.
/// If the entry already exists it is not added.
/// @param [in] container container to be added to
/// @param [in] entry element to be added
template <typename Container, typename Type = typename Container::value_type>
void add(Container& container, const Type& entry)
{
    auto iter = std::find(container.begin(), container.end(), entry);
    if (iter == container.end())
    {
        container.push_back(entry);
    }
}

/// Removes entry from a container if it exists. The entry type must provide operator==.
/// @param [in] container container to be removed from
/// @param [in] entry element to be removed
template <typename Container, typename Type = typename Container::value_type>
void remove(Container& container, const Type& entry)
{
    auto iter = std::find(container.begin(), container.end(), entry);
    if (iter != container.end())
    {
        container.erase(iter);
    }
}

/// Checks whether a container contains some specified entry. The entry type must provide operator==.
/// @param [in] container container to check
/// @param [in] entry element whose existence is checked
/// @return true if the container contains entry, false otherwise
template <typename Container, typename Type = typename Container::value_type>
bool hasElement(Container& container, const Type& entry)
{
    auto iter = std::find(container.begin(), container.end(), entry);
    return iter != container.end();
}

/// union of set1 and set2, set1 is the union after the operation and returned as result
/// @param [in] set1 first set operand for union operation, also holds the union result after operation
/// @param [in] set2 second set operand for union operation
/// @return reference to the union of both sets stored in set1
template <typename Container>
Container& unify(Container& set1, const Container& set2)
{
    for (auto& element : set2)
    {
        add(set1, element);
    }
    return set1;
}

} // namespace set
} // namespace cxx
} // namespace iox
