// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#if !defined(_WIN32) && !defined(__APPLE__)
#include "iceoryx_hoofs/testing/mocks/time_mock.hpp"
#include "iceoryx_hoofs/testing/mocks/mocks.hpp"

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables) Global variables are required for the mock handling
std::unique_ptr<time_MOCK> time_MOCK::mock;
bool time_MOCK::doUseMock = false;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

// NOLINTBEGIN(readability-inconsistent-declaration-parameter-name) Using __clk_id and __res would result in a warning for reserved identifier
// NOLINTBEGIN(bugprone-exception-escape) Intentional behavior for the mocks
#if defined(__linux__)
int clock_getres(clockid_t clk_id, struct timespec* res) noexcept
#else
int clock_getres(clockid_t clk_id, struct timespec* res)
#endif
{
    return (time_MOCK::doUseMock) ? time_MOCK::mock->clock_getres(clk_id, res)
                                  : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(clock_getres)(clk_id, res);
}

#if defined(__linux__)
int clock_gettime(clockid_t clk_id, struct timespec* res) noexcept
#else
int clock_gettime(clockid_t clk_id, struct timespec* res)
#endif
{
    return (time_MOCK::doUseMock) ? time_MOCK::mock->clock_gettime(clk_id, res)
                                  : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(clock_gettime)(clk_id, res);
}

#if defined(__linux__)
int clock_settime(clockid_t clk_id, const struct timespec* res) noexcept
#else
int clock_settime(clockid_t clk_id, const struct timespec* res)
#endif
{
    return (time_MOCK::doUseMock) ? time_MOCK::mock->clock_settime(clk_id, res)
                                  : STATIC_FUNCTION_LOADER_AUTO_DEDUCE(clock_settime)(clk_id, res);
}
// NOLINTEND(bugprone-exception-escape)
// NOLINTEND(readability-inconsistent-declaration-parameter-name)

#endif
