// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_TESTUTILS_EXPECT_NO_DEATH_HPP
#define IOX_HOOFS_TESTUTILS_EXPECT_NO_DEATH_HPP

/// @brief gtest offers EXPECT_DEATH as test but no alternative to test the opposite, that
///        the program does not terminate.
///        EXPECT_NO_DEATH offers to verify that something does not terminate.
/// @param[in] callable The callable which should not lead to termination
#define EXPECT_NO_DEATH(callable)                                                                                      \
    EXPECT_EXIT(                                                                                                       \
        {                                                                                                              \
            callable();                                                                                                \
            /* Use fprintf here instead of std::cerr. cerr can be rerouted to another stream which can lead to failing \
             * tests in combination with internal::CaptureStderr() for instance. */                                    \
            fprintf(stderr, "callable did not terminate");                                                             \
            exit(0);                                                                                                   \
        },                                                                                                             \
        ::testing::ExitedWithCode(0),                                                                                  \
        "callable did not terminate");

/// @brief gtest offers ASSERT_DEATH as test but no alternative to test the opposite, that
///        the program does not terminate.
///        ASSERT_NO_DEATH offers to verify that something does not terminate.
/// @param[in] callable The callable which should not lead to termination
#define ASSERT_NO_DEATH(callable)                                                                                      \
    ASSERT_EXIT(                                                                                                       \
        {                                                                                                              \
            callable();                                                                                                \
            /* Use fprintf here instead of std::cerr. cerr can be rerouted to another stream which can lead to failing \
             * tests in combination with internal::CaptureStderr() for instance. */                                    \
            fprintf(stderr, "callable did not terminate");                                                             \
            exit(0);                                                                                                   \
        },                                                                                                             \
        ::testing::ExitedWithCode(0),                                                                                  \
        "callable did not terminate");

#endif
