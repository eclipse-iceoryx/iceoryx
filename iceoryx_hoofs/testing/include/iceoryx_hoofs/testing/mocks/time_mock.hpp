// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT
#ifndef IOX_HOOFS_MOCKS_TIME_MOCK_HPP
#define IOX_HOOFS_MOCKS_TIME_MOCK_HPP

#if !defined(_WIN32) && !defined(__APPLE__)

#include "iceoryx_hoofs/testing/test.hpp"

#include <ctime>

using namespace ::testing;

class time_MOCK
{
  public:
    MOCK_METHOD2(clock_getres, int(clockid_t, struct timespec*));
    MOCK_METHOD2(clock_gettime, int(clockid_t, struct timespec*));
    MOCK_METHOD2(clock_settime, int(clockid_t, const struct timespec*));

    // NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables) Global variables are required for the mock handling
    static std::unique_ptr<time_MOCK> mock;
    static bool doUseMock; // = true;
    // NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)
};

//NOLINTBEGIN(readability-inconsistent-declaration-parameter-name) Using __clk_id and __res would result in a warning for reserved identifier
#if defined(__linux__)
int clock_getres(clockid_t clk_id, struct timespec* res) noexcept;
int clock_gettime(clockid_t clk_id, struct timespec* res) noexcept;
int clock_settime(clockid_t clk_id, const struct timespec* res) noexcept;
#else
int clock_getres(clockid_t clk_id, struct timespec* res);
int clock_gettime(clockid_t clk_id, struct timespec* res);
int clock_settime(clockid_t clk_id, const struct timespec* res);
#endif
//NOLINTEND(readability-inconsistent-declaration-parameter-name)

#endif // !defined(_WIN32) && !defined(__APPLE__)

#endif // IOX_HOOFS_MOCKS_TIME_MOCK_HPP
