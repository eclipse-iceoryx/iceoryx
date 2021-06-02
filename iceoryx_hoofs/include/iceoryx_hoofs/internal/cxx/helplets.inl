// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

namespace iox
{
namespace cxx
{
template <uint64_t StringCapacity>
inline bool isValidFileName(const string<StringCapacity>& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    uint64_t nameSize = name.size();

    const string<StringCapacity> currentDirectory(".");
    const string<StringCapacity> parentDirectory("..");

    if (name == currentDirectory || name == parentDirectory)
    {
        return false;
    }

    // check if the file contains only valid characters
    for (uint64_t i = 0; i < nameSize; ++i)
    {
        if (!((65 <= name.c_str()[i] && name.c_str()[i] <= 90) ||  // A-Z
              (97 <= name.c_str()[i] && name.c_str()[i] <= 122) || // a-z
              (48 <= name.c_str()[i] && name.c_str()[i] <= 57) ||  // 0-9
              name.c_str()[i] == 45 ||                             // -
              name.c_str()[i] == 46 ||                             // .
              name.c_str()[i] == 58 ||                             // :
              name.c_str()[i] == 95                                // _
              ))
        {
            return false;
        }
    }

    return true;
}

template <uint64_t StringCapacity>
inline bool isValidFilePath(const string<StringCapacity>& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    uint64_t nameSize = name.size();

    // a file path ends with the filename and not the path separator, only a
    // directory can end with a path separator
    auto numberOfPathSeparators = strlen(IOX_PATH_SEPARATORS);
    for (uint64_t i = 0; i < numberOfPathSeparators; ++i)
    {
        if (name.c_str()[nameSize - 1] == IOX_PATH_SEPARATORS[i])
        {
            return false;
        }
    }

    auto temp = name;

    const string<StringCapacity> currentDirectory(".");
    const string<StringCapacity> parentDirectory("..");

    while (!temp.empty())
    {
        auto separatorPosition = temp.find_first_of(IOX_PATH_SEPARATORS);

        // multiple slashes are explicitly allowed. the following paths
        // are equivalent:
        // /some/fuu/bar
        // //some///fuu////bar
        if (separatorPosition && *separatorPosition == 0)
        {
            temp = *temp.substr(*separatorPosition + 1);
            continue;
        }

        // verify if the entry between two path separators is a valid directory
        // name, e.g. either it has the relative component . or .. or conforms
        // with a valid file name
        if (separatorPosition)
        {
            auto filenameToVerify = temp.substr(0, *separatorPosition);
            if (!isValidFileName(*filenameToVerify) && *filenameToVerify != currentDirectory
                && *filenameToVerify != parentDirectory)
            {
                return false;
            }

            temp = *temp.substr(*separatorPosition + 1);
        }
        // we reached the last entry, if its a valid file name the path is valid
        else if (!separatorPosition)
        {
            return isValidFileName(temp);
        }
    }

    return false;
}
} // namespace cxx
} // namespace iox

#endif
