// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_ALGORITHM_INL
#define IOX_HOOFS_CXX_ALGORITHM_INL

#include "iceoryx_hoofs/cxx/algorithm.hpp"

namespace iox
{
namespace algorithm
{
template <typename T>
inline constexpr T maxVal(const T& left) noexcept
{
    return left;
}

template <typename T>
inline constexpr T maxVal(const T& left, const T& right) noexcept
{
    return (right < left) ? left : right;
}

template <typename T, typename... Targs>
inline constexpr T maxVal(const T& left, const T& right, const Targs&... args) noexcept
{
    return maxVal(maxVal(left, right), args...);
}

template <typename T>
inline constexpr T minVal(const T& left) noexcept
{
    return left;
}

template <typename T>
inline constexpr T minVal(const T& left, const T& right) noexcept
{
    return (left < right) ? left : right;
}

template <typename T, typename... Targs>
inline constexpr T minVal(const T& left, const T& right, const Targs&... args) noexcept
{
    return minVal(minVal(left, right), args...);
}

template <typename T, typename CompareType>
inline constexpr bool doesContainType() noexcept
{
    return std::is_same<T, CompareType>::value;
}

template <typename T, typename CompareType, typename Next, typename... Remainder>
inline constexpr bool doesContainType() noexcept
{
    return doesContainType<T, CompareType>() || doesContainType<T, Next, Remainder...>();
}

template <typename T>
inline constexpr bool doesContainValue(const T) noexcept
{
    return false;
}

template <typename T, typename... ValueList>
inline constexpr bool
doesContainValue(const T value, const T firstValueListEntry, const ValueList... remainingValueListEntries) noexcept
{
    // AXIVION Next Line AutosarC++19_03-M6.2.2 : intentional check for exact equality
    return (value == firstValueListEntry) ? true : doesContainValue(value, remainingValueListEntries...);
}

template <typename Container>
inline Container uniqueMergeSortedContainers(const Container& v1, const Container& v2) noexcept
{
    Container mergedContainer;
    uint64_t indexV1{0U};
    uint64_t indexV2{0U};
    const uint64_t v1Size{v1.size()};
    const uint64_t v2Size{v2.size()};

    while ((indexV1 < v1Size) && (indexV2 < v2Size))
    {
        if (v1[indexV1] == v2[indexV2])
        {
            IOX_DISCARD_RESULT(mergedContainer.emplace_back(v1[indexV1]));
            ++indexV1;
            ++indexV2;
        }
        else if (v1[indexV1] < v2[indexV2])
        {
            IOX_DISCARD_RESULT(mergedContainer.emplace_back(v1[indexV1]));
            ++indexV1;
        }
        else
        {
            IOX_DISCARD_RESULT(mergedContainer.emplace_back(v2[indexV2]));
            ++indexV2;
        }
    }

    while (indexV2 < v2Size)
    {
        IOX_DISCARD_RESULT(mergedContainer.emplace_back(v2[indexV2++]));
    }

    while (indexV1 < v1Size)
    {
        IOX_DISCARD_RESULT(mergedContainer.emplace_back(v1[indexV1++]));
    }

    return mergedContainer;
}


} // namespace algorithm
} // namespace iox

#endif // IOX_HOOFS_CXX_ALGORITHM_INL
