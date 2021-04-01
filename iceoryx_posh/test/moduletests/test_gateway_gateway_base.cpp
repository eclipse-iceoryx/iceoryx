// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

using namespace iox::runtime;
using iox::roudi::RouDiEnvironment;

using namespace iox::gw;

using iox::capro::IdString_t;

class GatewayBase_test : public TestWithParam<iox::capro::Interfaces>
{
  public:
    void SetUp(){};
    void TearDown(){};

    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};
    iox::runtime::PoshRuntime* m_senderRuntime{&iox::runtime::PoshRuntime::initRuntime("sender")};
    iox::gw::GatewayBase m_base{GetParam()};

    void InterOpWait() const
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    class GatewayBaseTestDestructor : public GatewayBase
    {
      public:
        GatewayBaseTestDestructor(const iox::capro::Interfaces f_interface) noexcept
            : GatewayBase(f_interface)
        {
        }

        iox::popo::InterfacePort* getInterfaceImpl()
        {
            return &m_interfaceImpl;
        }

        ~GatewayBaseTestDestructor() noexcept
        {
        }
    };
};

/// we require INSTANTIATE_TEST_CASE_P since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
INSTANTIATE_TEST_CASE_P(GatewayBasetests,
                        GatewayBase_test,
                        Values(iox::capro::Interfaces::SOMEIP,
                               iox::capro::Interfaces::INTERNAL));

#pragma GCC diagnostic pop

TEST_P(GatewayBase_test, InterfacePortWillBeDestroyedWhenGatewayGoesOutOfScope)
{
    iox::popo::InterfacePort* interfaceImpl;

    {
        GatewayBaseTestDestructor base{GetParam()};

        interfaceImpl = base.getInterfaceImpl();
    }

    EXPECT_TRUE(interfaceImpl->toBeDestroyed());
}

TEST_P(GatewayBase_test, GetCaProMessageMethodWithInvalidMessageReturnFalse)
{
    iox::capro::CaproMessage notValidCaproMessage;
    EXPECT_FALSE(m_base.getCaProMessage(notValidCaproMessage));
}

TEST_P(GatewayBase_test, GetCaProMessageMethodWithValidMessageReturnTrue)
{
    m_senderRuntime->offerService({"service1", "instance1"});
    this->InterOpWait();

    iox::popo::InterfacePort interfaceImpl{
        iox::runtime::PoshRuntime::getInstance().getMiddlewareInterface(GetParam())};
    this->InterOpWait();

    auto maybeCaproMessage = interfaceImpl.tryGetCaProMessage();
    iox::capro::CaproMessage ValidCaproMessage = maybeCaproMessage.value();

    EXPECT_TRUE(m_base.getCaProMessage(ValidCaproMessage));
}
