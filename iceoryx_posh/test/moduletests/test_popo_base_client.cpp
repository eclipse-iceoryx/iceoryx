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

#include "iceoryx_posh/internal/popo/base_client.hpp"
#include "iceoryx_posh/internal/popo/client_impl.hpp"
#include "iceoryx_posh/internal/popo/untyped_client_impl.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"
#include "iox/optional.hpp"
#include "mocks/client_mock.hpp"
#include "mocks/trigger_handle_mock.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::popo;
using namespace iox::runtime;
using ::testing::_;

using BaseClientWithMocks = BaseClient<MockClientPortUser, MockTriggeHandle>;
using UntypedClientWithMocks = iox::popo::UntypedClientImpl<BaseClientWithMocks>;
using TypedClientWithMocks = iox::popo::ClientImpl<uint64_t, uint64_t, BaseClientWithMocks>;

template <typename T>
int resetCallsFromDtors()
{
    // from derived and base class
    return 2;
}

template <>
int resetCallsFromDtors<BaseClientWithMocks>()
{
    // from base only
    return 1;
}

template <typename Base>
class TestBaseClient : public Base
{
  public:
    TestBaseClient(ServiceDescription sd, ClientOptions options)
        : Base(sd, options)
    {
    }

    using Base::port;

    using Base::disableEvent;
    using Base::disableState;
    using Base::enableEvent;
    using Base::enableState;
    using Base::getCallbackForIsStateConditionSatisfied;
    using Base::invalidateTrigger;
    using Base::m_trigger;
};

using BaseClientTypes = Types<BaseClientWithMocks, UntypedClientWithMocks, TypedClientWithMocks>;

TYPED_TEST_SUITE(BaseClient_test, BaseClientTypes, );

template <typename SutType>
class BaseClient_test : public Test
{
  public:
    using Sut = SutType;

    void SetUp() override
    {
        // we only need one non default option to check whether they are correctly passed to the underlying port
        options.nodeName = "engage";

        // the default ctor is used in the getMiddlewareClient call
        PortConfigInfo portInfo;
        MemoryManager memoryManager;
        ClientPortData portData{
            sd, runtimeName, roudi::DEFAULT_UNIQUE_ROUDI_ID, options, &memoryManager, portInfo.memoryInfo};
        EXPECT_CALL(*mockRuntime, getMiddlewareClient(sd, options, portInfo)).WillOnce(Return(&portData));

        sut.emplace(sd, options);
        EXPECT_CALL(this->sut->m_trigger, reset).WillRepeatedly(Return());
    }

    void TearDown() override
    {
        if (sut.has_value())
        {
            EXPECT_CALL(sut->port(), destroy).Times(1);
            sut.reset();
        }
    }

    iox::RuntimeName_t runtimeName{"HYPNOTOAD"};
    std::unique_ptr<PoshRuntimeMock> mockRuntime = PoshRuntimeMock::create(runtimeName);

    ServiceDescription sd{"make", "it", "so"};
    iox::popo::ClientOptions options;
    optional<TestBaseClient<Sut>> sut;
};

TYPED_TEST(BaseClient_test, DestructorCallsDestroyOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "fa8f6649-7889-41b1-867a-591cef414075");

    EXPECT_CALL(this->sut->port(), destroy).Times(1);

    this->sut.reset(); // reset from the optional calls the dtor of the inner type
}

TYPED_TEST(BaseClient_test, GetUidCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c1f401c-9ee2-40f9-8f97-2ae7dae594b3");

    const UniquePortId uid{roudi::DEFAULT_UNIQUE_ROUDI_ID};
    EXPECT_CALL(this->sut->port(), getUniqueID).WillOnce(Return(uid));

    EXPECT_THAT(this->sut->getUid(), Eq(uid));
}

TYPED_TEST(BaseClient_test, GetServiceDescriptionCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2d46bbe-479e-4c7b-9068-7c1003584c2f");

    EXPECT_CALL(this->sut->port(), getCaProServiceDescription).WillOnce(ReturnRef(this->sd));

    EXPECT_THAT(this->sut->getServiceDescription(), Eq(this->sd));
}

TYPED_TEST(BaseClient_test, ConnectCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e364583-c26b-4ba0-b55f-5121b4ed1b5f");

    EXPECT_CALL(this->sut->port(), connect).Times(1);

    this->sut->connect();
}

TYPED_TEST(BaseClient_test, GetConnectionStateCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "f093652b-421b-43e1-b69a-6bde15f18e6d");

    constexpr ConnectionState CONNECTION_STATE{ConnectionState::WAIT_FOR_OFFER};
    EXPECT_CALL(this->sut->port(), getConnectionState).WillOnce(Return(CONNECTION_STATE));

    EXPECT_THAT(this->sut->getConnectionState(), Eq(CONNECTION_STATE));
}

TYPED_TEST(BaseClient_test, DisconnectCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "025b478a-c9b7-4f08-821f-f3f4abdc6f65");

    EXPECT_CALL(this->sut->port(), disconnect).Times(1);

    this->sut->disconnect();
}

TYPED_TEST(BaseClient_test, HasResponsesCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d50f56a-a489-4c5c-9d17-c966fb7e171c");

    constexpr bool HAS_RESPONSES{true};
    EXPECT_CALL(this->sut->port(), hasNewResponses).WillOnce(Return(HAS_RESPONSES));

    EXPECT_THAT(this->sut->hasResponses(), Eq(HAS_RESPONSES));
}

TYPED_TEST(BaseClient_test, HasMissedResponsesCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a0a8bf6-47af-4ce4-acbb-adf7c09513f6");

    constexpr bool HAS_MISSED_RESPONSES{true};
    EXPECT_CALL(this->sut->port(), hasLostResponsesSinceLastCall).WillOnce(Return(HAS_MISSED_RESPONSES));

    EXPECT_THAT(this->sut->hasMissedResponses(), Eq(HAS_MISSED_RESPONSES));
}

TYPED_TEST(BaseClient_test, ReleaseQueuedResponsesCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "bd72358c-dc0c-4900-bea5-52be800f1448");

    EXPECT_CALL(this->sut->port(), releaseQueuedResponses).Times(1);

    this->sut->releaseQueuedResponses();
}

// BEGIN Listener and WaitSet related test

TYPED_TEST(BaseClient_test, InvalidateTriggerWithFittingTriggerIdCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "6a779c0c-a8b9-4b1c-a98a-5d074a63cea2");

    constexpr uint64_t TRIGGER_ID{13U};

    EXPECT_CALL(this->sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));
    EXPECT_CALL(this->sut->port(), unsetConditionVariable).Times(1);
    EXPECT_CALL(this->sut->m_trigger, invalidate).Times(1);

    this->sut->invalidateTrigger(TRIGGER_ID);
}

TYPED_TEST(BaseClient_test, InvalidateTriggerWithUnfittingTriggerIdDoesNotCallUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "98165eac-4a34-4dcc-b945-d2b60ff38541");

    constexpr uint64_t TRIGGER_ID_1{1U};
    constexpr uint64_t TRIGGER_ID_2{2U};

    EXPECT_CALL(this->sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID_2));
    EXPECT_CALL(this->sut->port(), unsetConditionVariable).Times(0);
    EXPECT_CALL(this->sut->m_trigger, invalidate).Times(0);

    this->sut->invalidateTrigger(TRIGGER_ID_1);
}

TYPED_TEST(BaseClient_test, EnableStateCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "43277404-5391-4d8f-a651-cad5ed50777c");

    for (const bool clientAttachedIndicator : {false, true})
    {
        SCOPED_TRACE(std::string("Test 'enableState' with client ")
                     + (clientAttachedIndicator ? "attached" : " not attached"));

        const uint64_t TRIGGER_ID{clientAttachedIndicator ? 42U : 73U};
        MockTriggeHandle triggerHandle;
        triggerHandle.triggerId = TRIGGER_ID;
        ConditionVariableData condVar{this->runtimeName};

        EXPECT_THAT(this->sut->m_trigger.triggerId, Ne(TRIGGER_ID));

        EXPECT_CALL(this->sut->m_trigger, operatorBoolMock).WillOnce(Return(clientAttachedIndicator));
        EXPECT_CALL(this->sut->m_trigger, getConditionVariableData).WillOnce(Return(&condVar));
        EXPECT_CALL(this->sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));

        EXPECT_CALL(this->sut->port(), setConditionVariable(Ref(condVar), TRIGGER_ID)).Times(1);

        this->sut->enableState(std::move(triggerHandle), ClientState::HAS_RESPONSE);

        EXPECT_THAT(this->sut->m_trigger.triggerId, Eq(TRIGGER_ID));

        if (clientAttachedIndicator)
        {
            IOX_TESTING_EXPECT_ERROR(
                iox::PoshError::
                    POPO__BASE_CLIENT_OVERRIDING_WITH_STATE_SINCE_HAS_RESPONSE_OR_RESPONSE_RECEIVED_ALREADY_ATTACHED);
        }
        else
        {
            IOX_TESTING_EXPECT_OK();
        }
    }
}

TYPED_TEST(BaseClient_test, GetCallbackForIsStateConditionSatisfiedReturnsCallbackToSelf)
{
    ::testing::Test::RecordProperty("TEST_ID", "8e0bcb91-e4fb-4129-a75a-92e1ef13add4");

    auto callback = this->sut->getCallbackForIsStateConditionSatisfied(ClientState::HAS_RESPONSE);

    constexpr bool HAS_RESPONSES{true};
    EXPECT_CALL(this->sut->port(), hasNewResponses).WillOnce(Return(HAS_RESPONSES));
    EXPECT_TRUE((*callback)());
}

TYPED_TEST(BaseClient_test, DisableStateCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e204a48-37e5-476c-b6b9-4f29a24302e9");

    EXPECT_CALL(this->sut->m_trigger, reset).Times(resetCallsFromDtors<typename TestFixture::Sut>());
    EXPECT_CALL(this->sut->port(), unsetConditionVariable).Times(1);

    this->sut->disableState(ClientState::HAS_RESPONSE);
}

TYPED_TEST(BaseClient_test, EnableEventCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "c78ad5f7-5e0b-4fad-86bf-75eb1d762010");

    for (const bool clientAttachedIndicator : {false, true})
    {
        SCOPED_TRACE(std::string("Test 'enableEvent' with client ")
                     + (clientAttachedIndicator ? "attached" : " not attached"));

        const uint64_t TRIGGER_ID{clientAttachedIndicator ? 42U : 73U};
        MockTriggeHandle triggerHandle;
        triggerHandle.triggerId = TRIGGER_ID;
        ConditionVariableData condVar{this->runtimeName};

        EXPECT_THAT(this->sut->m_trigger.triggerId, Ne(TRIGGER_ID));

        EXPECT_CALL(this->sut->m_trigger, operatorBoolMock).WillOnce(Return(clientAttachedIndicator));
        EXPECT_CALL(this->sut->m_trigger, getConditionVariableData).WillOnce(Return(&condVar));
        EXPECT_CALL(this->sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));

        EXPECT_CALL(this->sut->port(), setConditionVariable(Ref(condVar), TRIGGER_ID)).Times(1);

        this->sut->enableEvent(std::move(triggerHandle), ClientEvent::RESPONSE_RECEIVED);

        EXPECT_THAT(this->sut->m_trigger.triggerId, Eq(TRIGGER_ID));

        if (clientAttachedIndicator)
        {
            IOX_TESTING_EXPECT_ERROR(
                iox::PoshError::
                    POPO__BASE_CLIENT_OVERRIDING_WITH_EVENT_SINCE_HAS_RESPONSE_OR_RESPONSE_RECEIVED_ALREADY_ATTACHED);
        }
        else
        {
            IOX_TESTING_EXPECT_OK();
        }
    }
}

TYPED_TEST(BaseClient_test, DisableEventCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "c2f75387-d223-47df-a81c-7d7ab47b9b0d");

    EXPECT_CALL(this->sut->m_trigger, reset).Times(resetCallsFromDtors<typename TestFixture::Sut>());
    EXPECT_CALL(this->sut->port(), unsetConditionVariable).Times(1);

    this->sut->disableEvent(ClientEvent::RESPONSE_RECEIVED);
}

// END Listener and WaitSet related test

} // namespace
