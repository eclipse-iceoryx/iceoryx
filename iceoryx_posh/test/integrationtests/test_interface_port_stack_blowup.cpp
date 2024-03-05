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

#include "iceoryx_posh/gateway/gateway_base.hpp"

#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox::gw;
using namespace iox::roudi_env;
using namespace iox::testing;

class InterfacePortRequestStackBlowup_test : public RouDi_GTest
{
  public:
    InterfacePortRequestStackBlowup_test()
        : RouDi_GTest(MinimalIceoryxConfigBuilder().create())
    {
    }

    void SetUp(){};
    void TearDown(){};
};

TEST_F(InterfacePortRequestStackBlowup_test, RouDiMustContinue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d912182d-2a74-4056-be1d-19b538c10c9c");
    iox::runtime::PoshRuntime::initRuntime("interface_port_request_stack_blowup");
    GatewayBase sut(iox::capro::Interfaces::INTERNAL);
    iox::capro::CaproMessage caproMessage;
    // we don't care if there are capro messages or not, we just want to have a check that there was no segfault
    EXPECT_THAT(sut.getCaProMessage(caproMessage), AnyOf(true, false));
}

} // namespace
