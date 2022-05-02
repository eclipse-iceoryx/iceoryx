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

#ifndef IOX_HOOFS_CLI_TYPES_HPP
#define IOX_HOOFS_CLI_TYPES_HPP

#include "iceoryx_hoofs/cxx/string.hpp"
#include "iceoryx_hoofs/platform/platform_settings.hpp"

#include <cstdint>

namespace iox
{
namespace cli
{
enum class OptionType
{
    SWITCH,
    REQUIRED,
    OPTIONAL
};

enum class UnknownOption
{
    IGNORE,
    TERMINATE
};

static constexpr uint64_t MAX_OPTION_NAME_LENGTH = 32;
static constexpr uint64_t MAX_OPTION_ARGUMENT_LENGTH = 128;
static constexpr uint64_t MAX_OPTION_DESCRIPTION_LENGTH = 1024;

using OptionName_t = cxx::string<MAX_OPTION_NAME_LENGTH>;
using OptionDescription_t = cxx::string<MAX_OPTION_DESCRIPTION_LENGTH>;
using Argument_t = cxx::string<MAX_OPTION_ARGUMENT_LENGTH>;
using BinaryName_t = cxx::string<platform::IOX_MAX_PATH_LENGTH>;
} // namespace cli
} // namespace iox
#endif
