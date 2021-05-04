// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/thread.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace posix
{
void setThreadName(pthread_t thread, const ThreadName_t& name)
{
    auto result = cxx::makeSmartC(
        iox_pthread_setname_np, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, thread, name.c_str());

    // String length limit is ensured through cxx::string
    // ERANGE (string too long) intentionally not handled to avoid untestable and dead code
    cxx::Ensures(!result.hasErrors());
}

ThreadName_t getThreadName(pthread_t thread)
{
    char tempName[MAX_THREAD_NAME_LENGTH + 1U];

    auto result = cxx::makeSmartC(pthread_getname_np,
                                  cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE,
                                  {0},
                                  {},
                                  thread,
                                  tempName,
                                  MAX_THREAD_NAME_LENGTH + 1U);

    // String length limit is ensured through MAX_THREAD_NAME_LENGTH
    // ERANGE (string too small) intentionally not handled to avoid untestable and dead code
    cxx::Ensures(!result.hasErrors());

    return ThreadName_t(cxx::TruncateToCapacity, tempName);
}

} // namespace posix
} // namespace iox
