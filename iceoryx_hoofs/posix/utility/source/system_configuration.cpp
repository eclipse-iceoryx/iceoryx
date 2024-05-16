// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iox/detail/system_configuration.hpp"

#include "iox/assertions.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"

namespace iox
{
namespace detail
{
uint64_t pageSize() noexcept
{
    // sysconf fails when one provides an invalid name parameter. _SC_PAGESIZE
    // is a valid name parameter therefore it should never fail.
    return static_cast<uint64_t>(IOX_POSIX_CALL(iox_sysconf)(IOX_SC_PAGESIZE)
                                     .failureReturnValue(-1)
                                     .evaluate()
                                     .or_else([](auto& r) {
                                         IOX_LOG(FATAL, "This should never happen: " << r.getHumanReadableErrnum());
                                         IOX_PANIC("Internal logic error");
                                     })
                                     .value()
                                     .value);
}
} // namespace detail
} // namespace iox
