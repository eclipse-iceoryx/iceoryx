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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/application_port.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "test.hpp"


using namespace ::testing;
using namespace iox::capro;
using namespace iox::popo;


const iox::capro::ServiceDescription SERVICE_DESCRIPTION_VALID("Radar", "FrontRight", "ChuckNorrisDetected");
const iox::capro::ServiceDescription SERVICE_DESCRIPTION_EMPTY(0, 0, 0);
const iox::ProcessName_t APP_NAME_FOR_RECEIVER_PORTS = {"RecPort"};
const iox::ProcessName_t APP_NAME_FOR_SENDER_PORTS = {"SendPort"};
const iox::ProcessName_t APP_NAME_FOR_APPLICATION_PORTS = {"AppPort"};
const iox::ProcessName_t APP_NAME_FOR_INTERFACE_PORTS = {"InterfacePort"};
const iox::ProcessName_t APP_NAME_EMPTY = {""};

typedef BasePort* CreatePort();

BasePort* CreateCaProPort()
{
    BasePortData* basePortData = new BasePortData();
    return new BasePort(basePortData);
}

BasePort* CreateSenderPort()
{
    SenderPortData* senderPortData = new SenderPortData(SERVICE_DESCRIPTION_VALID, nullptr, APP_NAME_FOR_SENDER_PORTS);
    return new SenderPort(senderPortData);
}

BasePort* CreateReceiverPort()
{
    ReceiverPortData* receiverPortData = new ReceiverPortData(SERVICE_DESCRIPTION_VALID, APP_NAME_FOR_RECEIVER_PORTS);
    return new ReceiverPort(receiverPortData);
}

BasePort* CreateInterfacePort()
{
    InterfacePortData* interfacePortData =
        new InterfacePortData(APP_NAME_FOR_INTERFACE_PORTS, iox::capro::Interfaces::INTERNAL);
    return new InterfacePort(interfacePortData);
}

BasePort* CreateApplicationPort()
{
    ApplicationPortData* applicationPortData = new ApplicationPortData(APP_NAME_FOR_APPLICATION_PORTS);
    return new ApplicationPort(applicationPortData);
}

class BasePorttest : public Test
{
  public:
    BasePort sut;
    void SetUp(){};
    void TearDown(){};
};

class BasePortParamtest : public TestWithParam<CreatePort*>
{
    friend class BasePort;

  protected:
    BasePortParamtest()
        : sut((*GetParam())())
    {
    }
    ~BasePortParamtest()
    {
        uniquePortIds.emplace_back(sut->getUniqueID());
        delete sut;
    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};
    BasePort* sut;
    static std::vector<iox::UniquePortId> uniquePortIds;
};
std::vector<iox::UniquePortId> BasePortParamtest::uniquePortIds;

TEST_P(BasePortParamtest, getUniqueID)
{
    for (auto& id : uniquePortIds)
    {
        EXPECT_THAT(sut->getUniqueID(), Ne(id));
    }
}

TEST_P(BasePortParamtest, getCaProServiceDescription)
{
    if (this->GetParam() == CreateCaProPort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(SERVICE_DESCRIPTION_EMPTY));
    }
    else if (this->GetParam() == CreateReceiverPort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(SERVICE_DESCRIPTION_VALID));
    }
    else if (this->GetParam() == CreateSenderPort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(SERVICE_DESCRIPTION_VALID));
    }
    else if (this->GetParam() == CreateInterfacePort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(SERVICE_DESCRIPTION_EMPTY));
    }
    else if (this->GetParam() == CreateApplicationPort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(SERVICE_DESCRIPTION_EMPTY));
    }
    else
    {
        // We should not be here
        std::cout << "CaPro ServiceDescriptiontest test failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

TEST_P(BasePortParamtest, getApplicationname)
{
    if (this->GetParam() == CreateCaProPort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(APP_NAME_EMPTY));
    }
    else if (this->GetParam() == CreateReceiverPort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(APP_NAME_FOR_RECEIVER_PORTS));
    }
    else if (this->GetParam() == CreateSenderPort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(APP_NAME_FOR_SENDER_PORTS));
    }
    else if (this->GetParam() == CreateInterfacePort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(APP_NAME_FOR_INTERFACE_PORTS));
    }
    else if (this->GetParam() == CreateApplicationPort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(APP_NAME_FOR_APPLICATION_PORTS));
    }
    else
    {
        // We should not be here
        std::cout << "CaPro Applicationname test failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

/// we require INSTANTIATE_TEST_CASE since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
INSTANTIATE_TEST_CASE_P(
    CaPro,
    BasePortParamtest,
    Values(&CreateCaProPort, &CreateReceiverPort, &CreateSenderPort, &CreateInterfacePort, &CreateApplicationPort));
#pragma GCC diagnostic pop
