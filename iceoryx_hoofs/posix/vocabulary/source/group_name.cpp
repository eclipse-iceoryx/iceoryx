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

#include "iceoryx_platform/platform_settings.hpp"
#include "iox/string.hpp"

namespace iox
{
namespace details
{
bool group_name_does_contain_invalid_characters(const string<platform::MAX_GROUP_NAME_LENGTH>& value) noexcept
{
    for (uint64_t i = 0; i < value.size(); ++i)
    {
        const bool contains_a_to_z = 'a' <= value[i] && value[i] <= 'z';
        const bool contains_0_to_9 = '0' <= value[i] && value[i] <= '9';
        const bool contains_dash = value[i] == '-';

        if (!contains_a_to_z && !contains_0_to_9 && !contains_dash)
        {
            return true;
        }
    }

    return false;
}

bool group_name_does_contain_invalid_content(const string<platform::MAX_GROUP_NAME_LENGTH>& value) noexcept
{
    // group name is not allowed to be empty
    if (value.empty())
    {
        return true;
    }

    // a group name is not allowed to start with a number or dash
    if (value[0] == '-' || ('0' <= value[0] && value[0] <= '9'))
    {
        return true;
    }

    return false;
}
} // namespace details
} // namespace iox
