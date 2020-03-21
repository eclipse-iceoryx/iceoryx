// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "test.hpp"
#include "test_definitions.hpp"

using namespace ::testing;
using namespace iox::runtime;
using iox::roudi::RouDiEnvironment;

class Runtime_test : public Test
{
  public:
    Runtime_test()
    {
    }

    virtual ~Runtime_test()
    {
    }

    virtual void SetUp(){};

    virtual void TearDown(){};
};

TEST_F(Runtime_test, Appname_length_too_long)
{
    const std::string string104chars =
        "/MXIYXHyPF9KjXAPv9ev9jxofYDArZzTvf8FF5uaWWC4dwabcjW75DurqeN645IabAsXVfngor7784446vb4vhArwBxLZlN1k1Qmrtz";
    EXPECT_DEATH({ PoshRuntime::getInstance(string104chars); },
                 "Application name has more than 100 characters, including null termination!");
}

TEST_F(Runtime_test, Appname_length_ok)
{
    RouDiEnvironment m_roudiEnv(iox::RouDiConfig_t().setDefaults());
    const std::string string100chars =
        "/MXIYXHyPF9KjXAPv9ev9jxofYDArZzTvf8FF5uaWWC4dwabcjW75DurqeN645IabAsXVfngor7784446vb4vhArwBxLZlN1k1";
    EXPECT_NO_FATAL_FAILURE({ PoshRuntime::getInstance(string100chars); });
}

TEST_F(Runtime_test, Appname_empty)
{
    EXPECT_DEATH({ iox::runtime::PoshRuntime::getInstance(); },
                 "Cannot initialize runtime. Application name has not been specified!");
}

TEST_F(Runtime_test, no_Appname)
{
    const std::string wrong("");
    EXPECT_DEATH({ PoshRuntime::getInstance(wrong); },
                 "Cannot initialize runtime. Application name must not be empty!");
}

TEST_F(Runtime_test, no_leading_slash_Appname)
{
    const std::string wrong = "wrongname";
    EXPECT_DEATH({ PoshRuntime::getInstance(wrong); },
                 "Cannot initialize runtime. Application name wrongname does not have the required leading slash '/'");
}

TEST_F(Runtime_test, getInstanceName)
{
    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};
    const std::string appname = "/app";
    auto& sut = PoshRuntime::getInstance(appname);
    EXPECT_EQ(sut.getInstanceName(), appname);
}

/// @todo test the other public API of PoshRuntime
