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

#include "iceoryx_utils/posix_wrapper/thread.hpp"

namespace iox
{
namespace posix
{
cxx::expected<ThreadErrorType> setThreadName(pthread_t thread, const threadName_t& name)
{
    if (cxx::makeSmartC(pthread_setname_np, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, thread, name.c_str())
            .hasErrors())
    {
        return cxx::error<ThreadErrorType>(ThreadErrorType::EXCEEDED_RANGE_LIMIT);
    }
    else
    {
        return cxx::success<>();
    }
}

cxx::expected<ThreadErrorType> getThreadName(pthread_t thread, threadName_t& name)
{
    constexpr uint8_t maxCharCountThreadName = 17;
    char tempName[maxCharCountThreadName];
    if (cxx::makeSmartC(
            pthread_getname_np, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, thread, tempName, maxCharCountThreadName)
            .hasErrors())
    {
        return cxx::error<ThreadErrorType>(ThreadErrorType::EXCEEDED_RANGE_LIMIT);
    }
    else
    {
        name.assign(tempName);
        return cxx::success<>();
    }
}

} // namespace posix
} // namespace iox
