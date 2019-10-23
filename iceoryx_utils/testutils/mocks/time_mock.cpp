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

#include "time_mock.hpp"
#include "mocks.hpp"

std::unique_ptr<time_MOCK> time_MOCK::mock;
bool time_MOCK::doUseMock = false;

namespace time_orig
{
int (*clock_getres)(clockid_t,
                    struct timespec*) = mocks::assignSymbol<int (*)(clockid_t, struct timespec*)>("clock_getres");
int (*clock_gettime)(clockid_t,
                     struct timespec*) = mocks::assignSymbol<int (*)(clockid_t, struct timespec*)>("clock_gettime");
int (*clock_settime)(clockid_t, const struct timespec*) =
    mocks::assignSymbol<int (*)(clockid_t, const struct timespec*)>("clock_settime");
} // namespace time_orig

time_MOCK::time_MOCK()
{
}

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int clock_getres(clockid_t clk_id, struct timespec* res)
#else
int clock_getres(clockid_t clk_id, struct timespec* res) noexcept
#endif
{
    return (time_MOCK::doUseMock) ? time_MOCK::mock->clock_getres(clk_id, res) : time_orig::clock_getres(clk_id, res);
}
#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int clock_gettime(clockid_t clk_id, struct timespec* res)
#else
int clock_gettime(clockid_t clk_id, struct timespec* res) noexcept
#endif
{
    return (time_MOCK::doUseMock) ? time_MOCK::mock->clock_gettime(clk_id, res) : time_orig::clock_gettime(clk_id, res);
}

#if defined(QNX) || defined(QNX__) || defined(__QNX__)
int clock_settime(clockid_t clk_id, const struct timespec* res)
#else
int clock_settime(clockid_t clk_id, const struct timespec* res) noexcept
#endif
{
    return (time_MOCK::doUseMock) ? time_MOCK::mock->clock_settime(clk_id, res) : time_orig::clock_settime(clk_id, res);
}
