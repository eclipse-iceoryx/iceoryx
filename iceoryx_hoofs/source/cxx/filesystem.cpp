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
#include "iceoryx_hoofs/log/logstream.hpp"
#include <iostream>
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

template <typename StreamType>
StreamType& operator<<(StreamType& stream, perms value) noexcept
{
    if (value == perms::unknown)
    {
        stream << "unknown permissions";
        return stream;
    }

    bool hasPrecedingEntry = false;
    auto outputToStream = [&](const char* text) {
        if (hasPrecedingEntry)
        {
            stream << ", ";
        }
        hasPrecedingEntry = true;

        stream << text;
    };

    auto finishEntry = [&](bool isLastEntry = false) {
        if (hasPrecedingEntry)
        {
            stream << "}";
        }
        else
        {
            stream << "none}";
        }

        if (!isLastEntry)
        {
            stream << ",  ";
        }
        hasPrecedingEntry = false;
    };

    // owner
    stream << "owner: {";

    if ((value & perms::owner_read) != perms::none)
    {
        outputToStream("read");
    }

    if ((value & perms::owner_write) != perms::none)
    {
        outputToStream("write");
    }

    if ((value & perms::owner_exec) != perms::none)
    {
        outputToStream("execute");
    }

    finishEntry();

    // group
    stream << "group: {";

    if ((value & perms::group_read) != perms::none)
    {
        outputToStream("read");
    }

    if ((value & perms::group_write) != perms::none)
    {
        outputToStream("write");
    }

    if ((value & perms::group_exec) != perms::none)
    {
        outputToStream("execute");
    }

    finishEntry();

    // other
    stream << "others: {";

    if ((value & perms::others_read) != perms::none)
    {
        outputToStream("read");
    }

    if ((value & perms::others_write) != perms::none)
    {
        outputToStream("write");
    }

    if ((value & perms::others_exec) != perms::none)
    {
        outputToStream("execute");
    }

    finishEntry();

    // special bits
    stream << "special bits: {";
    if ((value & perms::set_uid) != perms::none)
    {
        outputToStream("set_uid");
    }

    if ((value & perms::set_gid) != perms::none)
    {
        outputToStream("set_git");
    }

    if ((value & perms::sticky_bit) != perms::none)
    {
        outputToStream("sticky_bit");
    }

    constexpr bool IS_LAST_ENTRY = true;
    finishEntry(IS_LAST_ENTRY);

    return stream;
}

template std::ostream& operator<<(std::ostream&, perms) noexcept;
template log::LogStream& operator<<(log::LogStream&, perms) noexcept;
} // namespace cxx
} // namespace iox
