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

#include "iceoryx_utils/internal/posix_wrapper/argv_inspection.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

/// @note This test should be called with multiple command line arguments!

// this global variable are set in main in test_utils_modules.cpp
extern int g_argc;
extern char** g_argv;

TEST(ArgvInspector_test, compare_arguments)
{
    iox::posix::ArgvInspector argvInspector;

    ASSERT_THAT(g_argc, Ge(1));

    int argNum = 0;
    for(int i = 0; argNum < g_argc; ++i)
    {

        std::string argUnderTest;
        ASSERT_TRUE(argvInspector.getCmdlineArgument(i, argUnderTest));

        // parameters starting with "--gtest" are filtered by gtest and not passed the the application
        // therefore the params differ from the ones in /proc/self/cmdline
        if(argUnderTest.rfind("--gtest", 0) == 0)
        {
            continue;
        }

        ASSERT_EQ(g_argv[argNum], argUnderTest);
        argNum++;
    }
}
