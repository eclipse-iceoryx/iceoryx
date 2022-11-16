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
#ifndef IOX_HOOFS_CXX_HELPLETS_INL
#define IOX_HOOFS_CXX_HELPLETS_INL

#include "iceoryx_hoofs/cxx/helplets.hpp"

namespace iox
{
namespace cxx
{
template <uint64_t StringCapacity>
inline bool isValidPathEntry(const string<StringCapacity>& name,
                             const RelativePathComponents relativePathComponents) noexcept
{
    const string<StringCapacity> currentDirectory{"."};
    const string<StringCapacity> parentDirectory{".."};

    if ((name == currentDirectory) || (name == parentDirectory))
    {
        return relativePathComponents == RelativePathComponents::ACCEPT;
    }

    const auto nameSize = name.size();

    for (uint64_t i{0}; i < nameSize; ++i)
    {
        // AXIVION Next Construct AutosarC++19_03-A3.9.1: Not used as an integer but as actual character
        const char c{name[i]};

        const bool isSmallLetter{(internal::ASCII_A <= c) && (c <= internal::ASCII_Z)};
        const bool isCapitalLetter{(internal::ASCII_CAPITAL_A <= c) && (c <= internal::ASCII_CAPITAL_Z)};
        const bool isNumber{(internal::ASCII_0 <= c) && (c <= internal::ASCII_9)};
        const bool isSpecialCharacter{((c == internal::ASCII_MINUS) || (c == internal::ASCII_DOT))
                                      || ((c == internal::ASCII_COLON) || (c == internal::ASCII_UNDERSCORE))};

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
inline bool isValidFileName(const string<StringCapacity>& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    // check if the file contains only valid characters
    return isValidPathEntry(name, RelativePathComponents::REJECT);
}

template <uint64_t StringCapacity>
inline bool isValidPathToFile(const string<StringCapacity>& name) noexcept
{
    if (doesEndWithPathSeparator(name))
    {
        return false;
    }

    string<StringCapacity> filePart{name};
    string<StringCapacity> pathPart;

    name.find_last_of(string<platform::IOX_NUMBER_OF_PATH_SEPARATORS>(TruncateToCapacity,
                                                                      &platform::IOX_PATH_SEPARATORS[0],
                                                                      platform::IOX_NUMBER_OF_PATH_SEPARATORS))
        .and_then([&](auto position) {
            name.substr(position + 1).and_then([&filePart](auto& s) { filePart = s; });
            name.substr(0, position).and_then([&pathPart](auto& s) { pathPart = s; });
        });

    return (pathPart.empty() || isValidPathToDirectory(pathPart)) && isValidFileName(filePart);
}

template <uint64_t StringCapacity>
inline bool isValidPathToDirectory(const string<StringCapacity>& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    auto temp = name;

    const string<StringCapacity> currentDirectory{"."};
    const string<StringCapacity> parentDirectory{".."};

    while (!temp.empty())
    {
        auto separatorPosition = temp.find_first_of(string<platform::IOX_NUMBER_OF_PATH_SEPARATORS>(
            TruncateToCapacity, &platform::IOX_PATH_SEPARATORS[0], platform::IOX_NUMBER_OF_PATH_SEPARATORS));

        // multiple slashes are explicitly allowed. the following paths
        // are equivalent:
        // /some/fuu/bar
        // //some///fuu////bar
        if (separatorPosition && (*separatorPosition == 0))
        {
            temp.substr(*separatorPosition + 1).and_then([&temp](auto& s) { temp = s; });
            continue;
        }

        // verify if the entry between two path separators is a valid directory
        // name, e.g. either it has the relative component . or .. or conforms
        // with a valid file name
        if (separatorPosition)
        {
            auto filenameToVerify = temp.substr(0, *separatorPosition);
            bool isValidDirectory{
                (isValidFileName(*filenameToVerify))
                || ((*filenameToVerify == currentDirectory) || (*filenameToVerify == parentDirectory))};
            if (!isValidDirectory)
            {
                return false;
            }

            temp.substr(*separatorPosition + 1).and_then([&temp](auto& s) { temp = s; });
        }
        // we reached the last entry, if its a valid file name the path is valid
        else
        {
            return isValidPathEntry(temp, RelativePathComponents::ACCEPT);
        }
    }

    return true;
}

template <uint64_t StringCapacity>
inline bool doesEndWithPathSeparator(const string<StringCapacity>& name) noexcept
{
    if (name.empty())
    {
        return false;
    }
    // AXIVION Next Construct AutosarC++19_03-A3.9.1: Not used as an integer but as actual character
    char lastCharacter{name[name.size() - 1U]};

    for (const auto separator : iox::platform::IOX_PATH_SEPARATORS)
    {
        if (lastCharacter == separator)
        {
            return true;
        }
    }
    return false;
}

template <typename F, typename T>
inline constexpr T from(const F) noexcept
{
    static_assert(always_false_v<F> && always_false_v<T>, "Conversion for the specified types is not implemented!\
    Please specialize 'template <typename F, typename T> constexpr T from(const F) noexcept'!");
}

template <typename T, typename F>
inline constexpr T into(const F value) noexcept
{
    return from<F, T>(value);
}
} // namespace cxx
} // namespace iox

#endif
