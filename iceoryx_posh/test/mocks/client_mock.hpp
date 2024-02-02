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

#ifndef IOX_POSH_MOCKS_CLIENT_MOCK_HPP
#define IOX_POSH_MOCKS_CLIENT_MOCK_HPP

#include "iceoryx_posh/internal/popo/base_client.hpp"
#include "iox/expected.hpp"
#include "mocks/base_port_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

class MockClientPortUser : public MockBasePort
{
  public:
    using MemberType_t = iox::popo::ClientPortData;
    explicit MockClientPortUser(MemberType_t&)
    {
    }
    MockClientPortUser() = default;
    ~MockClientPortUser() = default;

    MockClientPortUser(const MockClientPortUser&) = delete;
    MockClientPortUser& operator=(const MockClientPortUser&) = delete;
    MockClientPortUser(MockClientPortUser&&) noexcept
    {
    }
    MockClientPortUser& operator=(MockClientPortUser&&) noexcept
    {
        return *this;
    }

    MOCK_METHOD((iox::expected<iox::popo::RequestHeader*, iox::popo::AllocationError>),
                allocateRequest,
                (const uint64_t, const uint32_t),
                (noexcept));
    MOCK_METHOD(void, releaseRequest, (const iox::popo::RequestHeader* const), (noexcept));
    MOCK_METHOD((iox::expected<void, iox::popo::ClientSendError>),
                sendRequest,
                (iox::popo::RequestHeader* const),
                (noexcept));
    MOCK_METHOD(void, connect, (), (noexcept));
    MOCK_METHOD(void, disconnect, (), (noexcept));
    MOCK_METHOD(iox::ConnectionState, getConnectionState, (), (const, noexcept));
    MOCK_METHOD((iox::expected<const iox::popo::ResponseHeader*, iox::popo::ChunkReceiveResult>),
                getResponse,
                (),
                (noexcept));
    MOCK_METHOD(void, releaseResponse, (const iox::popo::ResponseHeader* const), (noexcept));
    MOCK_METHOD(void, releaseQueuedResponses, (), (noexcept));
    MOCK_METHOD(bool, hasNewResponses, (), (const, noexcept));
    MOCK_METHOD(bool, hasLostResponsesSinceLastCall, (), (noexcept));
    MOCK_METHOD(void, setConditionVariable, (iox::popo::ConditionVariableData&, const uint64_t), (noexcept));
    MOCK_METHOD(void, unsetConditionVariable, (), (noexcept));
    MOCK_METHOD(bool, isConditionVariableSet, (), (const, noexcept));
};

class MockBaseClient
{
  public:
    using PortType = MockClientPortUser;

    MockBaseClient(const iox::capro::ServiceDescription& sd, const iox::popo::ClientOptions& options) noexcept
        : serviceDescription(sd)
        , clientOptions(options)
    {
    }

    MOCK_METHOD(iox::popo::uid_t, getUid, (), (const, noexcept));
    MOCK_METHOD(const iox::capro::ServiceDescription&, getServiceDescription, (), (const, noexcept));
    MOCK_METHOD(void, connect, (), (noexcept));
    MOCK_METHOD(iox::ConnectionState, getConnectionState, (), (const, noexcept));
    MOCK_METHOD(void, disconnect, (), (noexcept));
    MOCK_METHOD(bool, hasResponses, (), (const, noexcept));
    MOCK_METHOD(bool, hasMissedResponses, (), (noexcept));
    MOCK_METHOD(void, releaseQueuedResponses, (), (noexcept));

    MOCK_METHOD(void, invalidateTrigger, (const uint64_t uniqueTriggerId), (noexcept));
    MOCK_METHOD(void, enableState, (iox::popo::TriggerHandle&&, const iox::popo::ClientState), (noexcept));
    MOCK_METHOD(iox::popo::WaitSetIsConditionSatisfiedCallback,
                getCallbackForIsStateConditionSatisfied,
                (const iox::popo::ClientState),
                (const, noexcept));
    MOCK_METHOD(void, disableState, (const iox::popo::ClientState), (noexcept));
    MOCK_METHOD(void, enableEvent, (iox::popo::TriggerHandle&&, const iox::popo::ClientEvent), (noexcept));
    MOCK_METHOD(void, disableEvent, (const iox::popo::ClientEvent), (noexcept));


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
    iox::popo::ClientOptions clientOptions;

    struct TriggerResetMock
    {
        void reset()
        {
        }
    };
    TriggerResetMock m_trigger;
};

#endif // IOX_POSH_MOCKS_CLIENT_MOCK_HPP
