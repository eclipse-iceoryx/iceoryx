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
template <uint64_t StringCapacity>
inline bool isValidPathEntry(const iox::string<StringCapacity>& name,
                             const RelativePathComponents relativePathComponents) noexcept
{
    const iox::string<StringCapacity> currentDirectory{"."};
    const iox::string<StringCapacity> parentDirectory{".."};

    if ((name == currentDirectory) || (name == parentDirectory))
    {
        return relativePathComponents == RelativePathComponents::ACCEPT;
    }

    const auto nameSize = name.size();

    for (uint64_t i{0}; i < nameSize; ++i)
    {
        // AXIVION Next Construct AutosarC++19_03-A3.9.1: Not used as an integer but as actual character
        const char c{name[i]};

        // AXIVION DISABLE STYLE FaultDetection-UnusedAssignments : False positive, variable IS used
        // AXIVION DISABLE STYLE AutosarC++19_03-A0.1.1 : False positive, variable IS used
        // AXIVION DISABLE STYLE AutosarC++19_03-M4.5.3 : We are explicitly checking for ASCII characters which have defined consecutive values
        const bool isSmallLetter{(internal::ASCII_A <= c) && (c <= internal::ASCII_Z)};
        const bool isCapitalLetter{(internal::ASCII_CAPITAL_A <= c) && (c <= internal::ASCII_CAPITAL_Z)};
        const bool isNumber{(internal::ASCII_0 <= c) && (c <= internal::ASCII_9)};
        const bool isSpecialCharacter{((c == internal::ASCII_DASH) || (c == internal::ASCII_DOT))
                                      || ((c == internal::ASCII_COLON) || (c == internal::ASCII_UNDERSCORE))};
        // AXIVION ENABLE STYLE AutosarC++19_03-M4.5.3
        // AXIVION ENABLE STYLE AutosarC++19_03-A0.1.1
        // AXIVION ENABLE STYLE FaultDetection-UnusedAssignments

        if ((!isSmallLetter && !isCapitalLetter) && (!isNumber && !isSpecialCharacter))
        {
            return false;
        }
    }

    if (nameSize == 0)
    {
        return true;
    }

    // dot at the end is invalid to be compatible with windows api
    return !(name[nameSize - 1] == '.');
}

template <uint64_t StringCapacity>
inline bool isValidFileName(const iox::string<StringCapacity>& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    // check if the file contains only valid characters
    return isValidPathEntry(name, RelativePathComponents::REJECT);
}

template <uint64_t StringCapacity>
inline bool isValidPathToFile(const iox::string<StringCapacity>& name) noexcept
{
    if (doesEndWithPathSeparator(name))
    {
        return false;
    }

    auto maybeSeparator = name.find_last_of(iox::string<platform::IOX_NUMBER_OF_PATH_SEPARATORS>(
        TruncateToCapacity, &platform::IOX_PATH_SEPARATORS[0], platform::IOX_NUMBER_OF_PATH_SEPARATORS));

    if (!maybeSeparator.has_value())
    {
        return isValidFileName(name);
    }

    const auto& position = maybeSeparator.value();

    bool isFileNameValid{false};
    name.substr(position + 1).and_then([&isFileNameValid](const auto& s) noexcept {
        isFileNameValid = isValidFileName(s);
    });

    bool isPathValid{false};
    name.substr(0, position).and_then([&isPathValid](const auto& s) noexcept {
        const bool isEmptyPath{s.empty()};
        const bool isPathToDirectoryValid{isValidPathToDirectory(s)};
        isPathValid = isEmptyPath || isPathToDirectoryValid;
    });

    // AXIVION Next Construct AutosarC++19_03-M0.1.2, AutosarC++19_03-M0.1.9, FaultDetection-DeadBranches : False positive! Branching depends on input parameter
    return isPathValid && isFileNameValid;
}

template <uint64_t StringCapacity>
inline bool isValidPathToDirectory(const iox::string<StringCapacity>& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    const iox::string<StringCapacity> currentDirectory{"."};
    const iox::string<StringCapacity> parentDirectory{".."};

    const iox::string<platform::IOX_NUMBER_OF_PATH_SEPARATORS> pathSeparators{
        TruncateToCapacity, &platform::IOX_PATH_SEPARATORS[0], platform::IOX_NUMBER_OF_PATH_SEPARATORS};

    auto remaining = name;
    while (!remaining.empty())
    {
        const auto separatorPosition = remaining.find_first_of(pathSeparators);

        if (separatorPosition.has_value())
        {
            const uint64_t position{separatorPosition.value()};

            // multiple slashes are explicitly allowed. the following paths
            // are equivalent:
            // /some/fuu/bar
            // //some///fuu////bar

            // verify if the entry between two path separators is a valid directory
            // name, e.g. either it has the relative component . or .. or conforms
            // with a valid file name
            if (position != 0)
            {
                const auto guaranteedSubstr = remaining.substr(0, position);
                const auto& filenameToVerify = guaranteedSubstr.value();
                const bool isValidDirectory{
                    (isValidFileName(filenameToVerify))
                    || ((filenameToVerify == currentDirectory) || (filenameToVerify == parentDirectory))};
                if (!isValidDirectory)
                {
                    return false;
                }
            }

            remaining.substr(position + 1).and_then([&remaining](const auto& s) noexcept { remaining = s; });
        }
        else // we reached the last entry, if its a valid file name the path is valid
        {
            return isValidPathEntry(remaining, RelativePathComponents::ACCEPT);
        }
    }

    return true;
}

// AXIVION Next Construct AutosarC++19_03-A5.2.5, AutosarC++19_03-M5.0.16, FaultDetection-OutOfBounds : IOX_PATH_SEPARATORS is not a string but an array of chars without a null termination and all elements are valid characters
template <uint64_t StringCapacity>
inline bool doesEndWithPathSeparator(const iox::string<StringCapacity>& name) noexcept
{
    if (name.empty())
    {
        return false;
    }
    // AXIVION Next Construct AutosarC++19_03-A3.9.1: Not used as an integer but as actual character
    const char lastCharacter{name[name.size() - 1U]};

    for (const auto separator : iox::platform::IOX_PATH_SEPARATORS)
    {
        if (lastCharacter == separator)
        {
            return true;
        }
    }
    return false;
}

constexpr access_rights::value_type access_rights::value() const noexcept
{
    return m_value;
}

constexpr bool operator==(const access_rights lhs, const access_rights rhs) noexcept
{
    return lhs.value() == rhs.value();
}

constexpr bool operator!=(const access_rights lhs, const access_rights rhs) noexcept
{
    return !(lhs == rhs);
}

constexpr access_rights operator|(const access_rights lhs, const access_rights rhs) noexcept
{
    return access_rights(lhs.value() | rhs.value());
}

constexpr access_rights operator&(const access_rights lhs, const access_rights rhs) noexcept
{
    return access_rights(lhs.value() & rhs.value());
}

constexpr access_rights operator^(const access_rights lhs, const access_rights rhs) noexcept
{
    return access_rights(lhs.value() ^ rhs.value());
}

constexpr access_rights operator~(const access_rights value) noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A4.7.1, AutosarC++19_03-M0.3.1, FaultDetection-IntegerOverflow : Cast is safe and required due to integer promotion
    return access_rights(static_cast<access_rights::value_type>(~value.value()));
}

constexpr access_rights operator|=(const access_rights lhs, const access_rights rhs) noexcept
{
    return operator|(lhs, rhs);
}

constexpr access_rights operator&=(const access_rights lhs, const access_rights rhs) noexcept
{
    return operator&(lhs, rhs);
}

constexpr access_rights operator^=(const access_rights lhs, const access_rights rhs) noexcept
{
    return operator^(lhs, rhs);
}
} // namespace iox

#endif
