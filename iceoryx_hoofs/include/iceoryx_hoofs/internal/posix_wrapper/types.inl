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
#ifndef IOX_HOOFS_POSIX_WRAPPER_TYPES_INL
#define IOX_HOOFS_POSIX_WRAPPER_TYPES_INL

#include "iceoryx_hoofs/posix_wrapper/types.hpp"

namespace iox
{
namespace posix
{
inline constexpr const char* asStringLiteral(const OpenMode mode) noexcept
{
    switch (mode)
    {
    case OpenMode::EXCLUSIVE_CREATE:
        return "OpenMode::EXCLUSIVE_CREATE";
    case OpenMode::PURGE_AND_CREATE:
        return "OpenMode::PURGE_AND_CREATE";
    case OpenMode::OPEN_OR_CREATE:
        return "OpenMode::OPEN_OR_CREATE";
    case OpenMode::OPEN_EXISTING:
        return "OpenMode::OPEN_EXISTING";
    }

    return "OpenMode::UNDEFINED_VALUE";
}

inline constexpr const char* asStringLiteral(const AccessMode mode) noexcept
{
    switch (mode)
    {
    case AccessMode::READ_ONLY:
        return "AccessMode::READ_ONLY";
    case AccessMode::READ_WRITE:
        return "AccessMode::READ_WRITE";
    case AccessMode::WRITE_ONLY:
        return "AccessMode::WRITE_ONLY";
    }

    return "AccessMode::UNDEFINED_VALUE";
}
} // namespace posix
} // namespace iox

#endif
