// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/runtime/posh_runtime_single_process.hpp"
#include "iceoryx_posh/testing/roudi_environment/roudi_environment.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::runtime;
using namespace iox::roudi;
using namespace iox;

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

    virtual void SetUp(){};

    virtual void TearDown(){};
};

TEST_F(PoshRuntimeSingleProcess_test, ConstructorPoshRuntimeSingleProcessIsSuccess)
{
    ::testing::Test::RecordProperty("TEST_ID", "9faf7053-86af-4d26-b3a7-fb3c6319ab86");
    iox::RouDiConfig_t defaultRouDiConfig = iox::RouDiConfig_t().setDefaults();
    IceOryxRouDiComponents roudiComponents(defaultRouDiConfig);

    RouDi roudi(roudiComponents.rouDiMemoryManager,
                roudiComponents.portManager,
                RouDi::RoudiStartupParameters{iox::roudi::MonitoringMode::OFF, false});

    const RuntimeName_t m_runtimeName{"App"};

    EXPECT_NO_FATAL_FAILURE({ PoshRuntimeSingleProcess m_runtimeSingleProcess(m_runtimeName); });
}

TEST_F(PoshRuntimeSingleProcess_test, ConstructorPoshRuntimeSingleProcessMultipleProcessIsFound)
{
    ::testing::Test::RecordProperty("TEST_ID", "1cc7ad5d-5878-454a-94ba-5cf412c22682");
    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};

    const RuntimeName_t m_runtimeName{"App"};

    iox::cxx::optional<iox::PoshError> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler<iox::PoshError>::setTemporaryErrorHandler(
        [&detectedError](const iox::PoshError error, const std::function<void()>, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        });

    PoshRuntimeSingleProcess m_runtimeSingleProcess(m_runtimeName);

    ASSERT_THAT(detectedError.has_value(), Eq(true));
    EXPECT_THAT(detectedError.value(), Eq(iox::PoshError::kPOSH__RUNTIME_IS_CREATED_MULTIPLE_TIMES));
}

} // namespace
