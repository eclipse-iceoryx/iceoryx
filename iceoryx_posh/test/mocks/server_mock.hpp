// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_MOCKS_SERVER_MOCK_HPP
#define IOX_POSH_MOCKS_SERVER_MOCK_HPP

#include "iceoryx_posh/internal/popo/base_server.hpp"
#include "iox/expected.hpp"
#include "mocks/base_port_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

class MockServerPortUser : public MockBasePort
{
  public:
    using MemberType_t = iox::popo::ServerPortData;
    explicit MockServerPortUser(MemberType_t&)
    {
    }
    MockServerPortUser() = default;
    ~MockServerPortUser() = default;

    MockServerPortUser(const MockServerPortUser&) = delete;
    MockServerPortUser& operator=(const MockServerPortUser&) = delete;
    MockServerPortUser(MockServerPortUser&&) noexcept
    {
    }
    MockServerPortUser& operator=(MockServerPortUser&&) noexcept
    {
        return *this;
    }

    MOCK_METHOD((iox::expected<const iox::popo::RequestHeader*, iox::popo::ServerRequestResult>),
                getRequest,
                (),
                (noexcept));
    MOCK_METHOD(void, releaseRequest, (const iox::popo::RequestHeader* const), (noexcept));
    MOCK_METHOD(void, releaseQueuedRequests, (), (noexcept));
    MOCK_METHOD(bool, hasNewRequests, (), (const, noexcept));
    MOCK_METHOD(bool, hasLostRequestsSinceLastCall, (), (noexcept));
    MOCK_METHOD((iox::expected<iox::popo::ResponseHeader*, iox::popo::AllocationError>),
                allocateResponse,
                (const iox::popo::RequestHeader* const, const uint64_t, const uint32_t),
                (noexcept));
    MOCK_METHOD(void, releaseResponse, (const iox::popo::ResponseHeader* const), (noexcept));
    MOCK_METHOD((iox::expected<void, iox::popo::ServerSendError>),
                sendResponse,
                (iox::popo::ResponseHeader* const),
                (noexcept));
    MOCK_METHOD(void, offer, (), (noexcept));
    MOCK_METHOD(void, stopOffer, (), (noexcept));
    MOCK_METHOD(bool, isOffered, (), (const, noexcept));
    MOCK_METHOD(bool, hasClients, (), (const, noexcept));
    MOCK_METHOD(void, setConditionVariable, (iox::popo::ConditionVariableData&, const uint64_t), (noexcept));
    MOCK_METHOD(void, unsetConditionVariable, (), (noexcept));
    MOCK_METHOD(bool, isConditionVariableSet, (), (const, noexcept));
};

class MockBaseServer
{
  public:
    using PortType = MockServerPortUser;

    MockBaseServer(const iox::capro::ServiceDescription& sd, const iox::popo::ServerOptions& options) noexcept
        : serviceDescription(sd)
        , serverOptions(options)
    {
    }

    MOCK_METHOD(iox::popo::uid_t, getUid, (), (const, noexcept));
    MOCK_METHOD(const iox::capro::ServiceDescription&, getServiceDescription, (), (const, noexcept));
    MOCK_METHOD(void, offer, (), (noexcept));
    MOCK_METHOD(void, stopOffer, (), (noexcept));
    MOCK_METHOD(bool, isOffered, (), (const, noexcept));
    MOCK_METHOD(bool, hasClients, (), (const, noexcept));
    MOCK_METHOD(bool, hasRequests, (), (const, noexcept));
    MOCK_METHOD(bool, hasMissedRequests, (), (noexcept));
    MOCK_METHOD(void, releaseQueuedRequests, (), (noexcept));

    MOCK_METHOD(void, invalidateTrigger, (const uint64_t uniqueTriggerId), (noexcept));
    MOCK_METHOD(void, enableState, (iox::popo::TriggerHandle&&, const iox::popo::ServerState), (noexcept));
    MOCK_METHOD(iox::popo::WaitSetIsConditionSatisfiedCallback,
                getCallbackForIsStateConditionSatisfied,
                (const iox::popo::ServerState),
                (const, noexcept));
    MOCK_METHOD(void, disableState, (const iox::popo::ServerState), (noexcept));
    MOCK_METHOD(void, enableEvent, (iox::popo::TriggerHandle&&, const iox::popo::ServerEvent), (noexcept));
    MOCK_METHOD(void, disableEvent, (const iox::popo::ServerEvent), (noexcept));


    const PortType& port() const noexcept
    {
        return mockPort;
    }

    PortType& port() noexcept
    {
        return mockPort;
    }

    PortType mockPort;
    iox::capro::ServiceDescription serviceDescription;
    iox::popo::ServerOptions serverOptions;

    struct TriggerResetMock
    {
        void reset()
        {
        }
    };
    TriggerResetMock m_trigger;
};

#endif // IOX_POSH_MOCKS_SERVER_MOCK_HPP
