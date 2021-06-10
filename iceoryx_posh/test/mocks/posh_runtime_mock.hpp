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

#ifndef IOX_POSH_MOCKS_POSH_RUNTIME_MOCK_HPP
#define IOX_POSH_MOCKS_POSH_RUNTIME_MOCK_HPP

#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <gmock/gmock.h>

using namespace ::testing;

class PoshRuntimeMock : public iox::runtime::PoshRuntime
{
  public:
    /// @todo iox-#841 simplify this when we switch to gmock v1.10
    MOCK_METHOD1(findServiceMock,
                 iox::cxx::expected<iox::runtime::InstanceContainer, iox::runtime::FindServiceError>(
                     const iox::capro::ServiceDescription&));
    MOCK_METHOD1(offerServiceMock, bool(const iox::capro::ServiceDescription&));
    MOCK_METHOD1(stopOfferServiceMock, void(const iox::capro::ServiceDescription&));
    MOCK_METHOD3(getMiddlewarePublisherMock,
                 iox::PublisherPortUserType::MemberType_t*(const iox::capro::ServiceDescription&,
                                                           const iox::popo::PublisherOptions&,
                                                           const iox::runtime::PortConfigInfo&));
    MOCK_METHOD3(getMiddlewareSubscriberMock,
                 iox::SubscriberPortUserType::MemberType_t*(const iox::capro::ServiceDescription&,
                                                            const iox::popo::SubscriberOptions&,
                                                            const iox::runtime::PortConfigInfo&));
    MOCK_METHOD2(getMiddlewareInterfaceMock,
                 iox::popo::InterfacePortData*(const iox::capro::Interfaces, const iox::NodeName_t&));
    MOCK_METHOD0(getMiddlewareApplicationMock, iox::popo::ApplicationPortData*());
    MOCK_METHOD0(getMiddlewareConditionVariableMock, iox::popo::ConditionVariableData*());
    MOCK_METHOD1(createNodeMock, iox::runtime::NodeData*(const iox::runtime::NodeProperty&));
    MOCK_METHOD0(getServiceRegistryChangeCounterMock, const std::atomic<uint64_t>*());
    MOCK_METHOD2(sendRequestToRouDiMock, bool(const iox::runtime::IpcMessage&, iox::runtime::IpcMessage&));

  protected:
    PoshRuntimeMock(const iox::RuntimeName_t& name)
        : iox::runtime::PoshRuntime(iox::cxx::optional<const iox::RuntimeName_t*>({&name}))
    {
    }

    iox::cxx::expected<iox::runtime::InstanceContainer, iox::runtime::FindServiceError>
    findService(const iox::capro::ServiceDescription& serviceDescription) noexcept override
    {
        return findServiceMock(serviceDescription);
    }

    bool offerService(const iox::capro::ServiceDescription& serviceDescription) noexcept override
    {
        return offerServiceMock(serviceDescription);
    }

    void stopOfferService(const iox::capro::ServiceDescription& serviceDescription) noexcept override
    {
        stopOfferServiceMock(serviceDescription);
    }

    iox::PublisherPortUserType::MemberType_t*
    getMiddlewarePublisher(const iox::capro::ServiceDescription& service,
                           const iox::popo::PublisherOptions& publisherOptions = {},
                           const iox::runtime::PortConfigInfo& portConfigInfo = {}) noexcept override
    {
        return getMiddlewarePublisherMock(service, publisherOptions, portConfigInfo);
    }

    iox::SubscriberPortUserType::MemberType_t*
    getMiddlewareSubscriber(const iox::capro::ServiceDescription& service,
                            const iox::popo::SubscriberOptions& subscriberOptions = {},
                            const iox::runtime::PortConfigInfo& portConfigInfo = {}) noexcept override
    {
        return getMiddlewareSubscriberMock(service, subscriberOptions, portConfigInfo);
    }

    iox::popo::InterfacePortData* getMiddlewareInterface(const iox::capro::Interfaces interface,
                                                         const iox::NodeName_t& nodeName = {}) noexcept override
    {
        return getMiddlewareInterfaceMock(interface, nodeName);
    }

    iox::popo::ApplicationPortData* getMiddlewareApplication() noexcept override
    {
        return getMiddlewareApplicationMock();
    }

    iox::popo::ConditionVariableData* getMiddlewareConditionVariable() noexcept override
    {
        return getMiddlewareConditionVariableMock();
    }

    iox::runtime::NodeData* createNode(const iox::runtime::NodeProperty& nodeProperty) noexcept override
    {
        return createNodeMock(nodeProperty);
    }

    const std::atomic<uint64_t>* getServiceRegistryChangeCounter() noexcept override
    {
        return getServiceRegistryChangeCounterMock();
    }

    bool sendRequestToRouDi(const iox::runtime::IpcMessage& msg, iox::runtime::IpcMessage& answer) noexcept override
    {
        return sendRequestToRouDiMock(msg, answer);
    }
};

#endif // IOX_POSH_MOCKS_CHUNK_MOCK_HPP
