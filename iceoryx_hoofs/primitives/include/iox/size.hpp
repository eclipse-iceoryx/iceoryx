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
#ifndef IOX_HOOFS_PRIMITIVES_SIZE_HPP
#define IOX_HOOFS_PRIMITIVES_SIZE_HPP

#include <cstdint>

namespace iox
{
/// @brief Returns container.size(), converted to the return type if necessary.
/// @tparam Container Type of the container
/// @param container A container or view with a size member function
/// @return The size of container
template <typename Container>
constexpr auto size(const Container& container) -> decltype(container.size())
{
    return container.size();
}

/// @brief Get the capacity of a C array at compile time
/// @code
/// constexpr uint32_t FOO[42]{};
/// IOX_LOG(INFO, size(FOO)); // will print 42
/// @endcode
/// @tparam T the type of the array filled out by the compiler.
/// @tparam CapacityValue the capacity of the array filled out by the compiler.
/// @param[in] The actual content of the array is not of interest. Its just the capacity of the array that matters.
/// @return Returns the capacity of the array at compile time.
template <typename T, uint64_t CapacityValue>
// AXIVION Next Construct AutosarC++19_03-A2.10.5, AutosarC++19_03-M17.0.3 : The function is in the 'iox' namespace which prevents easy misuse
// AXIVION Next Construct AutosarC++19_03-A18.1.1 : returning capacity of C array at compile time is safe, no possibility of out of bounds access
// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
static constexpr uint64_t size(T const (&/*notInterested*/)[CapacityValue]) noexcept
{
    return CapacityValue;
}
} // namespace iox
#endif // IOX_HOOFS_PRIMITIVES_SIZE_HPP
