// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/thread.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

namespace iox
{
namespace posix
{
void setThreadName(pthread_t thread, const ThreadName_t& name) noexcept
{
    posixCall(iox_pthread_setname_np)(thread, name.c_str()).successReturnValue(0).evaluate().or_else([](auto& r) {
        // String length limit is ensured through cxx::string
        // ERANGE (string too long) intentionally not handled to avoid untestable and dead code
        std::cerr << "This should never happen! " << r.getHumanReadableErrnum() << std::endl;
        cxx::Ensures(false && "internal logic error");
    });
}

ThreadName_t getThreadName(pthread_t thread) noexcept
{
    char tempName[MAX_THREAD_NAME_LENGTH + 1U];

    posixCall(pthread_getname_np)(thread, tempName, MAX_THREAD_NAME_LENGTH + 1U)
        .successReturnValue(0)
        .evaluate()
        .or_else([](auto& r) {
            // String length limit is ensured through MAX_THREAD_NAME_LENGTH
            // ERANGE (string too small) intentionally not handled to avoid untestable and dead code
            std::cerr << "This should never happen! " << r.getHumanReadableErrnum() << std::endl;
            cxx::Ensures(false && "internal logic error");
        });

    return ThreadName_t(cxx::TruncateToCapacity, tempName);
}

} // namespace posix
} // namespace iox