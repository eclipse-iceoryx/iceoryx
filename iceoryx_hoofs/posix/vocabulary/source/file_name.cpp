// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#include "iox/file_name.hpp"
#include "iox/filesystem.hpp"

namespace iox
{
namespace details
{
bool file_name_does_contain_invalid_characters(const string<platform::IOX_MAX_FILENAME_LENGTH>& value) noexcept
{
    const auto valueSize = value.size();

    for (uint64_t i{0}; i < valueSize; ++i)
    {
        // AXIVION Next Construct AutosarC++19_03-A3.9.1: Not used as an integer but as actual character
        const char c{value.unchecked_at(i)};

        const bool isSmallLetter{internal::ASCII_A <= c && c <= internal::ASCII_Z};
        const bool isCapitalLetter{internal::ASCII_CAPITAL_A <= c && c <= internal::ASCII_CAPITAL_Z};
        const bool isNumber{internal::ASCII_0 <= c && c <= internal::ASCII_9};
        const bool isSpecialCharacter{c == internal::ASCII_DASH || c == internal::ASCII_DOT
                                      || c == internal::ASCII_COLON || c == internal::ASCII_UNDERSCORE};

        if ((!isSmallLetter && !isCapitalLetter) && (!isNumber && !isSpecialCharacter))
        {
            return true;
        }
    }

    return false;
}

bool file_name_does_contain_invalid_content(const string<platform::IOX_MAX_FILENAME_LENGTH>& value) noexcept
{
    return (value.empty() || value == "." || value == "..");
}
} // namespace details
} // namespace iox
