// Copyright (c) 2019, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/roudi/iceoryx_roudi_app.hpp"
#include "iceoryx_posh/roudi/roudi_cmd_line_parser_config_file_option.hpp"
#include "iceoryx_posh/roudi/roudi_config_toml_file_provider.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
using ::testing::Return;

using iox::roudi::IceOryxRouDiApp;

namespace iox
{
namespace test
{

class IceoryxRoudiApp_Child : public IceOryxRouDiApp
{
    public:

    IceoryxRoudiApp_Child(const config::CmdLineParser& cmdLineParser, const RouDiConfig_t& roudiConfig):IceOryxRouDiApp(cmdLineParser,roudiConfig)
    {
    }

    bool getVariableMRun()
    {
        return m_run;
    }

    void setVariableMRun(bool condition)
    {
        m_run = condition;
    }

    uint8_t callRun() noexcept
    {
        return run();
    }
};

/// @brief Test goal: This file tests class roudi app
class IceoryxRoudiApp_test : public Test
{
  public:

    iox::config::CmdLineParserConfigFileOption cmdLineParser;
    int argc;
    char* argv[];

    virtual void setUp()
    {
        cmdLineParser.parse(argc, argv);
    }
};

TEST_F(IceoryxRoudiApp_test, CheckConstructorIsSuccessfull)
{
    IceoryxRoudiApp_Child roudi(cmdLineParser, iox::RouDiConfig_t().setDefaults());

    EXPECT_TRUE(roudi.getVariableMRun());
}

TEST_F(IceoryxRoudiApp_test, CheckRunMethodWithMRunFalseReturnExitSuccess)
{
    IceoryxRoudiApp_Child roudi(cmdLineParser, iox::RouDiConfig_t().setDefaults());

    roudi.setVariableMRun(false);

    uint8_t result = roudi.callRun();

    EXPECT_EQ(result, EXIT_SUCCESS);
}

} // namespace test
} // namespace iox