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

#include "iceoryx_utils/posix_wrapper/pthread.hpp"

namespace iox
{
namespace posix
{
cxx::expected<PThreadErrorType> setThreadName(pthread_t thread, const char* name)
{
    cxx::string<16> truncatedName{cxx::TruncateToCapacity, name};

    if (cxx::makeSmartC(
            pthread_setname_np, cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, thread, truncatedName.c_str())
            .hasErrors())
    {
        return cxx::error<PThreadErrorType>(PThreadErrorType::EXCEEDED_RANGE_LIMIT);
    }
    else
    {
        return cxx::success<>();
    }
}

} // namespace posix
} // namespace iox
