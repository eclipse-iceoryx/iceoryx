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

template <typename StringType>
inline bool isFileName(const StringType& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    uint64_t nameSize = name.size();

    // a single dot is the relative path to the current directory and therefore
    // no file name
    if (nameSize == 1 && name[0] == '.')
    {
        return false;
    }

    // a double dot is the relative path to the directory above and therefore
    // no file name
    if (nameSize == 2 && name[0] == '.' && name[1] == '.')
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

template <typename StringType>
inline bool isFilePath(const StringType& name) noexcept
{
    if (name.empty())
    {
        return false;
    }

    uint64_t nameSize = name.size();

    // a file path ends with the filename and not the path separator, only a
    // directory can end with a path separator
    if (name[nameSize - 1] == IOX_PATH_SEPARATOR)
    {
        return false;
    }

    return true;
}


#endif
