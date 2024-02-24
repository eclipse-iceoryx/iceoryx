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

#include "iceoryx_posh/internal/popo/base_server.hpp"
#include "iceoryx_posh/internal/popo/server_impl.hpp"
#include "iceoryx_posh/internal/popo/untyped_server_impl.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"
#include "iox/optional.hpp"
#include "mocks/server_mock.hpp"
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

using BaseServerWithMocks = BaseServer<MockServerPortUser, MockTriggeHandle>;
using UntypedServerWithMocks = iox::popo::UntypedServerImpl<BaseServerWithMocks>;
using TypedServerWithMocks = iox::popo::ServerImpl<uint64_t, uint64_t, BaseServerWithMocks>;

template <typename T>
int resetCallsFromDtors()
{
    // from derived and base class
    return 2;
}

template <>
int resetCallsFromDtors<BaseServerWithMocks>()
{
    // from base only
    return 1;
}

template <typename Base>
class TestBaseServer : public Base
{
  public:
    TestBaseServer(ServiceDescription sd, ServerOptions options)
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

using BaseServerTypes = Types<BaseServerWithMocks, UntypedServerWithMocks, TypedServerWithMocks>;

TYPED_TEST_SUITE(BaseServer_test, BaseServerTypes, );

template <typename SutType>
class BaseServer_test : public Test
{
  public:
    using Sut = SutType;

    void SetUp() override
    {
        // we only need one non default option to check whether they are correctly passed to the underlying port
        options.nodeName = "engage";

        // the default ctor is used in the getMiddlewareServer call
        PortConfigInfo portInfo;
        MemoryManager memoryManager;
        ServerPortData portData{
            sd, runtimeName, roudi::DEFAULT_UNIQUE_ROUDI_ID, options, &memoryManager, portInfo.memoryInfo};
        EXPECT_CALL(*mockRuntime, getMiddlewareServer(sd, options, portInfo)).WillOnce(Return(&portData));

        sut.emplace(sd, options);
        EXPECT_CALL(this->sut->m_trigger, reset).WillRepeatedly(Return());
    }

    void TearDown() override
    {
        if (sut.has_value())
        {
            EXPECT_CALL(this->sut->port(), destroy).Times(1);
            sut.reset();
        }
    }

    iox::RuntimeName_t runtimeName{"HYPNOTOAD"};
    std::unique_ptr<PoshRuntimeMock> mockRuntime = PoshRuntimeMock::create(runtimeName);

    ServiceDescription sd{"make", "it", "so"};
    iox::popo::ServerOptions options;
    optional<TestBaseServer<Sut>> sut;
};

TYPED_TEST(BaseServer_test, DestructorCallsDestroyOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "731fbc24-e4de-4223-af6b-baae6b87463d");

    EXPECT_CALL(this->sut->port(), destroy).Times(1);

    this->sut.reset(); // reset from the optional calls the dtor of the inner type
}

TYPED_TEST(BaseServer_test, GetUidCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ff821b6-4977-4405-b95d-60fb84933d28");

    const UniquePortId uid{roudi::DEFAULT_UNIQUE_ROUDI_ID};
    EXPECT_CALL(this->sut->port(), getUniqueID).WillOnce(Return(uid));

    EXPECT_THAT(this->sut->getUid(), Eq(uid));
}

TYPED_TEST(BaseServer_test, GetServiceDescriptionCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "1481bfe5-4c70-4960-bb4c-92c637e2ec79");

    EXPECT_CALL(this->sut->port(), getCaProServiceDescription).WillOnce(ReturnRef(this->sd));

    EXPECT_THAT(this->sut->getServiceDescription(), Eq(this->sd));
}

TYPED_TEST(BaseServer_test, OfferCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b50c652-4821-4fc2-abb4-d942db704894");

    EXPECT_CALL(this->sut->port(), offer).Times(1);

    this->sut->offer();
}

TYPED_TEST(BaseServer_test, StopOfferCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "57e19e6e-a649-4e16-8cbc-7c7d922c0100");

    EXPECT_CALL(this->sut->port(), stopOffer).Times(1);

    this->sut->stopOffer();
}

TYPED_TEST(BaseServer_test, IsOfferedCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4b46a37-5331-4306-9df1-b092ea2d62ef");

    constexpr bool IS_OFFERED{true};
    EXPECT_CALL(this->sut->port(), isOffered).WillOnce(Return(IS_OFFERED));

    EXPECT_THAT(this->sut->isOffered(), Eq(IS_OFFERED));
}

TYPED_TEST(BaseServer_test, HasClientsCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c2eb6f3-5ce6-4bba-88de-08fcff87f5cf");

    constexpr bool HAS_CLIENTS{true};
    EXPECT_CALL(this->sut->port(), hasClients).WillOnce(Return(HAS_CLIENTS));

    EXPECT_THAT(this->sut->hasClients(), Eq(HAS_CLIENTS));
}

TYPED_TEST(BaseServer_test, HasRequestsCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "bcc738d3-21f7-4f36-9395-4a89f2f88a07");

    constexpr bool HAS_REQUESTS{true};
    EXPECT_CALL(this->sut->port(), hasNewRequests).WillOnce(Return(HAS_REQUESTS));

    EXPECT_THAT(this->sut->hasRequests(), Eq(HAS_REQUESTS));
}

TYPED_TEST(BaseServer_test, HasMissedRequestsCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0567e45-f66d-410b-b744-efdd4e566464");

    constexpr bool HAS_MISSED_REQUESTS{true};
    EXPECT_CALL(this->sut->port(), hasLostRequestsSinceLastCall).WillOnce(Return(HAS_MISSED_REQUESTS));

    EXPECT_THAT(this->sut->hasMissedRequests(), Eq(HAS_MISSED_REQUESTS));
}

TYPED_TEST(BaseServer_test, ReleaseQueuedRequestsCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a226bd2-5c33-436a-adc6-d59e24de1074");

    EXPECT_CALL(this->sut->port(), releaseQueuedRequests).Times(1);

    this->sut->releaseQueuedRequests();
}

// BEGIN Listener and WaitSet related test

TYPED_TEST(BaseServer_test, InvalidateTriggerWithFittingTriggerIdCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "fab216c1-b88f-4755-b5d4-7cf0fb95bc5a");

    constexpr uint64_t TRIGGER_ID{13U};

    EXPECT_CALL(this->sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));
    EXPECT_CALL(this->sut->port(), unsetConditionVariable).Times(1);
    EXPECT_CALL(this->sut->m_trigger, invalidate).Times(1);

    this->sut->invalidateTrigger(TRIGGER_ID);
}

TYPED_TEST(BaseServer_test, InvalidateTriggerWithUnfittingTriggerIdDoesNotCallUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "a895a258-1237-4de6-ab85-7246e3404d3a");

    constexpr uint64_t TRIGGER_ID_1{1U};
    constexpr uint64_t TRIGGER_ID_2{2U};

    EXPECT_CALL(this->sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID_2));
    EXPECT_CALL(this->sut->port(), unsetConditionVariable).Times(0);
    EXPECT_CALL(this->sut->m_trigger, invalidate).Times(0);

    this->sut->invalidateTrigger(TRIGGER_ID_1);
}

TYPED_TEST(BaseServer_test, EnableStateCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "e97beefa-f83d-42c5-8087-02bf4b9f2a32");

    for (const bool serverAttachedIndicator : {false, true})
    {
        SCOPED_TRACE(std::string("Test 'enableState' with server ")
                     + (serverAttachedIndicator ? "attached" : " not attached"));

        const uint64_t TRIGGER_ID{serverAttachedIndicator ? 42U : 73U};
        MockTriggeHandle triggerHandle;
        triggerHandle.triggerId = TRIGGER_ID;
        ConditionVariableData condVar{this->runtimeName};

        EXPECT_THAT(this->sut->m_trigger.triggerId, Ne(TRIGGER_ID));

        EXPECT_CALL(this->sut->m_trigger, operatorBoolMock).WillOnce(Return(serverAttachedIndicator));
        EXPECT_CALL(this->sut->m_trigger, getConditionVariableData).WillOnce(Return(&condVar));
        EXPECT_CALL(this->sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));

        EXPECT_CALL(this->sut->port(), setConditionVariable(Ref(condVar), TRIGGER_ID)).Times(1);

        this->sut->enableState(std::move(triggerHandle), ServerState::HAS_REQUEST);

        EXPECT_THAT(this->sut->m_trigger.triggerId, Eq(TRIGGER_ID));

        if (serverAttachedIndicator)
        {
            IOX_TESTING_EXPECT_ERROR(
                iox::PoshError::
                    POPO__BASE_SERVER_OVERRIDING_WITH_STATE_SINCE_HAS_REQUEST_OR_REQUEST_RECEIVED_ALREADY_ATTACHED);
        }
        else
        {
            IOX_TESTING_EXPECT_OK();
        }
    }
}

TYPED_TEST(BaseServer_test, GetCallbackForIsStateConditionSatisfiedReturnsCallbackToSelf)
{
    ::testing::Test::RecordProperty("TEST_ID", "7f9d8e30-ae60-4f68-9961-ad36b4fa9bae");

    auto callback = this->sut->getCallbackForIsStateConditionSatisfied(ServerState::HAS_REQUEST);

    constexpr bool HAS_REQUESTS{true};
    EXPECT_CALL(this->sut->port(), hasNewRequests).WillOnce(Return(HAS_REQUESTS));
    EXPECT_TRUE((*callback)());
}

TYPED_TEST(BaseServer_test, DisableStateCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "ce85051e-f18c-4c0f-a5c9-4c2701c4bb30");

    EXPECT_CALL(this->sut->m_trigger, reset).Times(resetCallsFromDtors<typename TestFixture::Sut>());
    EXPECT_CALL(this->sut->port(), unsetConditionVariable).Times(1);

    this->sut->disableState(ServerState::HAS_REQUEST);
}

TYPED_TEST(BaseServer_test, EnableEventCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5715e02-7362-4d4e-a387-11367b804ce1");

    for (const bool serverAttachedIndicator : {false, true})
    {
        SCOPED_TRACE(std::string("Test 'enableEvent' with server ")
                     + (serverAttachedIndicator ? "attached" : " not attached"));

        const uint64_t TRIGGER_ID{serverAttachedIndicator ? 42U : 73U};
        MockTriggeHandle triggerHandle;
        triggerHandle.triggerId = TRIGGER_ID;
        ConditionVariableData condVar{this->runtimeName};

        EXPECT_THAT(this->sut->m_trigger.triggerId, Ne(TRIGGER_ID));

        EXPECT_CALL(this->sut->m_trigger, operatorBoolMock).WillOnce(Return(serverAttachedIndicator));
        EXPECT_CALL(this->sut->m_trigger, getConditionVariableData).WillOnce(Return(&condVar));
        EXPECT_CALL(this->sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));

        EXPECT_CALL(this->sut->port(), setConditionVariable(Ref(condVar), TRIGGER_ID)).Times(1);

        this->sut->enableEvent(std::move(triggerHandle), ServerEvent::REQUEST_RECEIVED);

        EXPECT_THAT(this->sut->m_trigger.triggerId, Eq(TRIGGER_ID));

        if (serverAttachedIndicator)
        {
            IOX_TESTING_EXPECT_ERROR(
                iox::PoshError::
                    POPO__BASE_SERVER_OVERRIDING_WITH_EVENT_SINCE_HAS_REQUEST_OR_REQUEST_RECEIVED_ALREADY_ATTACHED);
        }
        else
        {
            IOX_TESTING_EXPECT_OK();
        }
    }
}

TYPED_TEST(BaseServer_test, DisableEventCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d7bee13-e654-4048-a57a-f7ba94b614b1");

    EXPECT_CALL(this->sut->m_trigger, reset).Times(resetCallsFromDtors<typename TestFixture::Sut>());
    EXPECT_CALL(this->sut->port(), unsetConditionVariable).Times(1);

    this->sut->disableEvent(ServerEvent::REQUEST_RECEIVED);
}

// END Listener and WaitSet related test

} // namespace
