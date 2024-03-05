// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iox/scope_guard.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::capro;
using namespace iox::popo;

const iox::capro::ServiceDescription SERVICE_DESCRIPTION("Radar", "FrontRight", "ChuckNorrisDetected");
const iox::capro::ServiceDescription DEFAULT_SERVICE_DESCRIPTION;

const iox::RuntimeName_t RUNTIME_NAME_FOR_BASE_PORTS = {"BasePort"};
const iox::RuntimeName_t RUNTIME_NAME_FOR_PUBLISHER_PORTS = {"PublisherPort"};
const iox::RuntimeName_t RUNTIME_NAME_FOR_SUBSCRIBER_PORTS = {"SubscriberPort"};
const iox::RuntimeName_t RUNTIME_NAME_FOR_CLIENT_PORTS = {"ClientPort"};
const iox::RuntimeName_t RUNTIME_NAME_FOR_SERVER_PORTS = {"ServerPort"};
const iox::RuntimeName_t RUNTIME_NAME_FOR_INTERFACE_PORTS = {"InterfacePort"};

iox::mepoo::MemoryManager m_memoryManager;
std::vector<UniquePortId> uniquePortIds;

using PortDataTypes =
    Types<BasePortData, PublisherPortData, SubscriberPortData, ClientPortData, ServerPortData, InterfacePortData>;

TYPED_TEST_SUITE(BasePort_test, PortDataTypes, );

// port data factories

template <typename T>
T* createPortData()
{
    return nullptr;
}
template <>
BasePortData* createPortData()
{
    return new BasePortData(SERVICE_DESCRIPTION, RUNTIME_NAME_FOR_BASE_PORTS, iox::roudi::DEFAULT_UNIQUE_ROUDI_ID);
}
template <>
PublisherPortData* createPortData()
{
    PublisherOptions options;
    options.historyCapacity = 1U;
    return new PublisherPortData(SERVICE_DESCRIPTION,
                                 RUNTIME_NAME_FOR_PUBLISHER_PORTS,
                                 iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                 &m_memoryManager,
                                 options);
}
template <>
SubscriberPortData* createPortData()
{
    return new SubscriberPortData(SERVICE_DESCRIPTION,
                                  RUNTIME_NAME_FOR_SUBSCRIBER_PORTS,
                                  iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                  iox::popo::VariantQueueTypes::FiFo_MultiProducerSingleConsumer,
                                  SubscriberOptions());
}
template <>
ClientPortData* createPortData()
{
    ClientOptions options;
    options.responseQueueCapacity = 1U;
    return new ClientPortData(SERVICE_DESCRIPTION,
                              RUNTIME_NAME_FOR_CLIENT_PORTS,
                              iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                              options,
                              &m_memoryManager);
}
template <>
ServerPortData* createPortData()
{
    ServerOptions options;
    options.requestQueueCapacity = 13U;
    return new ServerPortData(SERVICE_DESCRIPTION,
                              RUNTIME_NAME_FOR_SERVER_PORTS,
                              iox::roudi::DEFAULT_UNIQUE_ROUDI_ID,
                              options,
                              &m_memoryManager);
}
template <>
InterfacePortData* createPortData()
{
    return new InterfacePortData(
        RUNTIME_NAME_FOR_INTERFACE_PORTS, iox::roudi::DEFAULT_UNIQUE_ROUDI_ID, iox::capro::Interfaces::INTERNAL);
}

// expected ServiceDescription factories

template <typename T>
const ServiceDescription& expectedServiceDescription()
{
    return SERVICE_DESCRIPTION;
}
template <>
const ServiceDescription& expectedServiceDescription<PublisherPortData>()
{
    return SERVICE_DESCRIPTION;
}
template <>
const ServiceDescription& expectedServiceDescription<SubscriberPortData>()
{
    return SERVICE_DESCRIPTION;
}
template <>
const ServiceDescription& expectedServiceDescription<ClientPortData>()
{
    return SERVICE_DESCRIPTION;
}
template <>
const ServiceDescription& expectedServiceDescription<ServerPortData>()
{
    return SERVICE_DESCRIPTION;
}
template <>
const ServiceDescription& expectedServiceDescription<InterfacePortData>()
{
    return DEFAULT_SERVICE_DESCRIPTION;
}

// expected ProcessName factories

template <typename T>
const iox::RuntimeName_t& expectedProcessName()
{
    return RUNTIME_NAME_FOR_BASE_PORTS;
}
template <>
const iox::RuntimeName_t& expectedProcessName<PublisherPortData>()
{
    return RUNTIME_NAME_FOR_PUBLISHER_PORTS;
}
template <>
const iox::RuntimeName_t& expectedProcessName<SubscriberPortData>()
{
    return RUNTIME_NAME_FOR_SUBSCRIBER_PORTS;
}
template <>
const iox::RuntimeName_t& expectedProcessName<ClientPortData>()
{
    return RUNTIME_NAME_FOR_CLIENT_PORTS;
}
template <>
const iox::RuntimeName_t& expectedProcessName<ServerPortData>()
{
    return RUNTIME_NAME_FOR_SERVER_PORTS;
}
template <>
const iox::RuntimeName_t& expectedProcessName<InterfacePortData>()
{
    return RUNTIME_NAME_FOR_INTERFACE_PORTS;
}

template <typename PortData>
class BasePort_test : public Test
{
  public:
    using PortData_t = PortData;

    virtual void SetUp()
    {
        for (auto& id : uniquePortIds)
        {
            EXPECT_THAT(this->sut.getUniqueID(), Ne(id));
        }
        uniquePortIds.emplace_back(sut.getUniqueID());
    }

    virtual void TearDown()
    {
    }

    std::unique_ptr<PortData> sutData{createPortData<PortData>()};
    BasePort sut{sutData.get()};
};

TYPED_TEST(BasePort_test, CallingGetCaProServiceDescriptionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb52f436-8ca4-46fd-8ae6-1518086898bc");
    using PortData_t = typename TestFixture::PortData_t;
    EXPECT_THAT(this->sut.getCaProServiceDescription(), Eq(expectedServiceDescription<PortData_t>()));
}

TYPED_TEST(BasePort_test, CallingGetRuntimeNameWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5df7c7cb-efe0-4ae7-9da1-5a5c977b5c22");
    using PortData_t = typename TestFixture::PortData_t;
    EXPECT_THAT(this->sut.getRuntimeName(), Eq(expectedProcessName<PortData_t>()));
}

} // namespace
