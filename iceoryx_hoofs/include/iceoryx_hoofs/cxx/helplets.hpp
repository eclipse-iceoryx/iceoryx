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
#ifndef IOX_HOOFS_CXX_HELPLETS_HPP
#define IOX_HOOFS_CXX_HELPLETS_HPP

#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iox/string.hpp"

#include "iceoryx_platform/platform_correction.hpp"
#include "iceoryx_platform/platform_settings.hpp"

namespace iox
{
namespace cxx
{
template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value, void>::type>
struct not_null
{
  public:
    // this class should behave like a pointer which never can be nullptr, adding explicit
    // would defeat the purpose
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    not_null(T t) noexcept
        : m_value(t)
    {
        Expects(t != nullptr);
    }

    // AXIVION Next Construct AutosarC++19_03-A13.5.2,AutosarC++19_03-A13.5.3:this should behave like a pointer which never can be nullptr,
    // adding explicit would defeat the purpose
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator T() const noexcept
    {
        return m_value;
    }

  private:
    T m_value;
};

/// @brief Get the capacity of a C array at compile time
/// @code
/// constexpr uint32_t FOO[42]{};
/// IOX_LOG(INFO) << arrayCapacity(FOO); // will print 42
/// @endcode
/// @tparam T the type of the array filled out by the compiler.
/// @tparam CapacityValue the capacity of the array filled out by the compiler.
/// @param[in] The actual content of the array is not of interest. Its just the capacity of the array that matters.
/// @return Returns the capacity of the array at compile time.
template <typename T, uint64_t CapacityValue>
// AXIVION Next Construct AutosarC++19_03-A18.1.1:returning capacity of C array at compile time is safe, no
// possibility of out of bounds access
// NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
static constexpr uint64_t arrayCapacity(T const (&/*notInterested*/)[CapacityValue]) noexcept
{
    return CapacityValue;
}

/// @brief Returns info whether called on a 32-bit system
/// @return True if called on 32-bit, false if not 32-bit system
constexpr bool isCompiledOn32BitSystem() noexcept
{
    return INTPTR_MAX == INT32_MAX;
}
} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_HELPLETS_HPP
