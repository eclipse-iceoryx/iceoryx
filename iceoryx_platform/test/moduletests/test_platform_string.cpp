// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_platform/string.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

TEST(STRING_test, strerror_r_of_known_erron_works)
{
    ::testing::Test::RecordProperty("TEST_ID", "1fbf88c9-07c4-4959-9127-18a78db27c22");
    constexpr size_t BUFFER_SIZE{1024};
    char buf[BUFFER_SIZE];
    buf[0] = 0;

    auto* error_string = iox_gnu_strerror_r(ENOENT, &buf[0], BUFFER_SIZE);

    EXPECT_THAT(error_string, StrEq("No such file or directory"));
}

TEST(STRING_test, strerror_r_of_unknown_error_works_when_buffer_is_large_enough)
{
    ::testing::Test::RecordProperty("TEST_ID", "92213d70-61b7-47dd-beac-c86ce25b6991");
    constexpr size_t BUFFER_SIZE{1024};
    char buf[BUFFER_SIZE];
    buf[0] = 0;

    auto* error_string = iox_gnu_strerror_r(123456789, &buf[0], BUFFER_SIZE);

    // on Linux this is 'Unknown error 123456789', on macOS 'Unknown error: 123456789' and on Windows 'Unknown error'
    EXPECT_THAT(error_string, HasSubstr("Unknown error"));
}

TEST(STRING_test, strerror_r_of_unknown_error_is_truncated_when_buffer_is_too_small)
{
    ::testing::Test::RecordProperty("TEST_ID", "1dbba291-575f-495b-a174-0859b79980b1");
    constexpr size_t BUFFER_SIZE{10};
    char buf[BUFFER_SIZE];
    buf[0] = 0;

    auto* error_string = iox_gnu_strerror_r(123456789, &buf[0], BUFFER_SIZE);

    EXPECT_THAT(error_string, StrEq("Unknown e"));
}

} // namespace
