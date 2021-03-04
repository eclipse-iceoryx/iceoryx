// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_UTILS_POSIX_WRAPPER_SYSTEM_CONFIGURATION_HPP
#define IOX_UTILS_POSIX_WRAPPER_SYSTEM_CONFIGURATION_HPP

#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/platform/unistd.hpp"

#include <cstdint>

namespace iox
{
namespace posix
{
constexpr uint64_t MaxPageSize = 1024u * 512u;

cxx::optional<uint64_t> pageSize();

} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_SYSTEM_CONFIGURATION_HPP
