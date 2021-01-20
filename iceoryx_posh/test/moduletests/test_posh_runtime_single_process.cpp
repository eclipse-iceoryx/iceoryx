// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#
#include "iceoryx_posh/runtime/posh_runtime_single_process.hpp"
#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox::runtime;
using namespace iox::roudi;

namespace iox
{
namespace test
{
/// @brief Test goal: This test suit verifies class posh_runtime_single_process

class PoshRuntimeSingleProcess_test : public Test
{
  public:
    PoshRuntimeSingleProcess_test()
    {
    }

    virtual ~PoshRuntimeSingleProcess_test()
    {
    }

    virtual void SetUp()
    {
    };

    virtual void TearDown()
    {
    };   
};

TEST_F(PoshRuntimeSingleProcess_test, ConstructorPoshRuntimeSingleProcessIsSuccess)
{
  iox::RouDiConfig_t defaultRouDiConfig = iox::RouDiConfig_t().setDefaults();
  IceOryxRouDiComponents roudiComponents(defaultRouDiConfig);

  RouDi roudi(roudiComponents.m_rouDiMemoryManager,
                            roudiComponents.m_portManager,
                            RouDi::RoudiStartupParameters{iox::roudi::MonitoringMode::OFF, false});

  const ProcessName_t& m_runtimeName{"App"};
  PoshRuntimeSingleProcess m_runtimeSingleProcess(m_runtimeName);
}

TEST_F(PoshRuntimeSingleProcess_test, ConstructorPoshRuntimeSingleProcessMultipleProcessIsFound)
{
  RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};

  const ProcessName_t& m_runtimeName{"App"};

  EXPECT_DEATH({PoshRuntimeSingleProcess m_runtimeSingleProcess(m_runtimeName);}, ".*");
}

} // namespace test
} // namespace iox 
