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
#ifndef IOX_UTILS_POSIX_WRAPPER_PTHREAD_HPP
#define IOX_UTILS_POSIX_WRAPPER_PTHREAD_HPP

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/platform/pthread.hpp"

namespace iox
{
namespace posix
{
constexpr uint64_t MAX_THREAD_NAME_LENGTH = 15U;

using ThreadName_t = cxx::string<MAX_THREAD_NAME_LENGTH>;

void setThreadName(pthread_t thread, const ThreadName_t& name);
ThreadName_t getThreadName(pthread_t thread);

} // namespace posix
} // namespace iox

#endif // IOX_UTILS_POSIX_WRAPPER_PTHREAD_HPP
