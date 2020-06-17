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
#include "iceoryx_utils/cxx/helplets.hpp"
#include "test.hpp"


using namespace ::testing;
using namespace iox::capro;
using namespace iox::popo;
using CString100 = iox::cxx::CString100;


const iox::capro::ServiceDescription m_servicedesc("Radar", "FrontRight", "ChuckNorrisDetected");
const iox::capro::ServiceDescription m_emptyservicedesc(0, 0, 0);
CString100 m_receiverportname = {"RecPort"};
CString100 m_senderportname = {"SendPort"};
CString100 m_applicationportname = {"AppPort"};
CString100 m_interfaceportname = {"InterfacePort"};
CString100 m_emptyappname = {""};
typedef BasePort* CreatePort();

BasePort* CreateCaProPort()
{
    BasePortData* basePortData = new BasePortData();
    return new BasePort(basePortData);
}

BasePort* CreateSenderPort()
{
    SenderPortData* senderPortData = new SenderPortData(m_servicedesc, nullptr, "SendPort");
    return new SenderPort(senderPortData);
}

BasePort* CreateReceiverPort()
{
    ReceiverPortData* receiverPortData = new ReceiverPortData(m_servicedesc, "RecPort");
    return new ReceiverPort(receiverPortData);
}

BasePort* CreateInterfacePort()
{
    InterfacePortData* interfacePortData = new InterfacePortData("InterfacePort", iox::capro::Interfaces::INTERNAL);
    return new InterfacePort(interfacePortData);
}

BasePort* CreateApplicationPort()
{
    ApplicationPortData* applicationPortData = new ApplicationPortData("AppPort");
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
        delete sut;
    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    BasePort* sut;
};

TEST_P(BasePortParamtest, getUniqueID)
{
    if (this->GetParam() == CreateCaProPort)
    {
        // we have already created 5 CaProPorts in the previous test, so we expect as uniqueid = 6
        EXPECT_THAT(sut->getUniqueID(), Ne(0u));
    }
    else if (this->GetParam() == CreateReceiverPort)
    {
        EXPECT_THAT(sut->getUniqueID(), Ne(0u));
    }
    else if (this->GetParam() == CreateSenderPort)
    {
        EXPECT_THAT(sut->getUniqueID(), Ne(0u));
    }
    else if (this->GetParam() == CreateInterfacePort)
    {
        EXPECT_THAT(sut->getUniqueID(), Ne(0u));
    }
    else if (this->GetParam() == CreateApplicationPort)
    {
        EXPECT_THAT(sut->getUniqueID(), Ne(0u));
    }
    else
    {
        // We should not be here
        std::cout << "CaPro UniqueId test failed!" << std::endl;
        exit(EXIT_FAILURE);
    }
}

TEST_P(BasePortParamtest, getCaProServiceDescription)
{
    if (this->GetParam() == CreateCaProPort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(m_emptyservicedesc));
    }
    else if (this->GetParam() == CreateReceiverPort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(m_servicedesc));
    }
    else if (this->GetParam() == CreateSenderPort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(m_servicedesc));
    }
    else if (this->GetParam() == CreateInterfacePort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(m_emptyservicedesc));
    }
    else if (this->GetParam() == CreateApplicationPort)
    {
        EXPECT_THAT(sut->getCaProServiceDescription(), Eq(m_emptyservicedesc));
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
        EXPECT_THAT(sut->getProcessName(), Eq(m_emptyappname));
    }
    else if (this->GetParam() == CreateReceiverPort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(m_receiverportname));
    }
    else if (this->GetParam() == CreateSenderPort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(m_senderportname));
    }
    else if (this->GetParam() == CreateInterfacePort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(m_interfaceportname));
    }
    else if (this->GetParam() == CreateApplicationPort)
    {
        EXPECT_THAT(sut->getProcessName(), Eq(m_applicationportname));
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
