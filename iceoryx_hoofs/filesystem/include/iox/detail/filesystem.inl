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
#ifndef IOX_HOOFS_FILESYSTEM_FILESYSTEM_INL
#define IOX_HOOFS_FILESYSTEM_FILESYSTEM_INL

#include "iox/filesystem.hpp"

namespace iox
{
inline constexpr const char* asStringLiteral(const OpenMode mode) noexcept
{
    switch (mode)
    {
    case OpenMode::ExclusiveCreate:
        return "OpenMode::ExclusiveCreate";
    case OpenMode::PurgeAndCreate:
        return "OpenMode::PurgeAndCreate";
    case OpenMode::OpenOrCreate:
        return "OpenMode::OpenOrCreate";
    case OpenMode::OpenExisting:
        return "OpenMode::OpenExisting";
    }

    return "OpenMode::UndefinedValue";
}

inline constexpr const char* asStringLiteral(const AccessMode mode) noexcept
{
    switch (mode)
    {
    case AccessMode::ReadOnly:
        return "AccessMode::ReadOnly";
    case AccessMode::ReadWrite:
        return "AccessMode::ReadWrite";
    case AccessMode::WriteOnly:
        return "AccessMode::WriteOnly";
    }

    return "AccessMode::UndefinedValue";
}


inline constexpr access_rights::value_type access_rights::value() const noexcept
{
    return m_value;
}

inline constexpr bool operator==(const access_rights lhs, const access_rights rhs) noexcept
{
    return lhs.value() == rhs.value();
}

inline constexpr bool operator!=(const access_rights lhs, const access_rights rhs) noexcept
{
    return !(lhs == rhs);
}

inline constexpr access_rights operator|(const access_rights lhs, const access_rights rhs) noexcept
{
    return access_rights(lhs.value() | rhs.value());
}

inline constexpr access_rights operator&(const access_rights lhs, const access_rights rhs) noexcept
{
    return access_rights(lhs.value() & rhs.value());
}

inline constexpr access_rights operator^(const access_rights lhs, const access_rights rhs) noexcept
{
    return access_rights(lhs.value() ^ rhs.value());
}

inline constexpr access_rights operator~(const access_rights value) noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A4.7.1, AutosarC++19_03-M0.3.1, FaultDetection-IntegerOverflow : Cast is safe and required due to integer promotion
    return access_rights(static_cast<access_rights::value_type>(~value.value()));
}

inline constexpr access_rights operator|=(const access_rights lhs, const access_rights rhs) noexcept
{
    return operator|(lhs, rhs);
}

inline constexpr access_rights operator&=(const access_rights lhs, const access_rights rhs) noexcept
{
    return operator&(lhs, rhs);
}

inline constexpr access_rights operator^=(const access_rights lhs, const access_rights rhs) noexcept
{
    return operator^(lhs, rhs);
}
} // namespace iox

#endif
