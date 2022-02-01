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

#include "iceoryx_hoofs/cxx/filesystem.hpp"
#include <type_traits>

namespace iox
{
namespace cxx
{
perms operator|(const perms& lhs, const perms& rhs) noexcept
{
    using T = std::underlying_type<perms>::type;
    return static_cast<perms>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

perms operator&(const perms& lhs, const perms& rhs) noexcept
{
    using T = std::underlying_type<perms>::type;
    return static_cast<perms>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

perms operator^(const perms& lhs, const perms& rhs) noexcept
{
    using T = std::underlying_type<perms>::type;
    return static_cast<perms>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

perms operator~(const perms& value) noexcept
{
    using T = std::underlying_type<perms>::type;
    return static_cast<perms>(~static_cast<T>(value));
}

perms operator|=(perms& lhs, const perms& rhs) noexcept
{
    return lhs = lhs | rhs;
}

perms operator&=(perms& lhs, const perms& rhs) noexcept
{
    return lhs = lhs & rhs;
}

perms operator^=(perms& lhs, const perms& rhs) noexcept
{
    return lhs = lhs ^ rhs;
}
} // namespace cxx
} // namespace iox

