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

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/internal/popo/base_server.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"
#include "mocks/server_mock.hpp"
#include "mocks/trigger_handle_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::capro;
using namespace iox::cxx;
using namespace iox::mepoo;
using namespace iox::popo;
using namespace iox::runtime;
using ::testing::_;

using BaseServerWithMocks = BaseServer<MockServerPortUser, MockTriggeHandle>;

class TestBaseServer : public BaseServerWithMocks
{
  public:
    TestBaseServer(ServiceDescription sd, ServerOptions options)
        : BaseServerWithMocks::BaseServer(sd, options)
    {
    }

    using BaseServerWithMocks::port;

    using BaseServerWithMocks::disableEvent;
    using BaseServerWithMocks::disableState;
    using BaseServerWithMocks::enableEvent;
    using BaseServerWithMocks::enableState;
    using BaseServerWithMocks::getCallbackForIsStateConditionSatisfied;
    using BaseServerWithMocks::invalidateTrigger;
    using BaseServerWithMocks::m_trigger;
};

class BaseServer_test : public Test
{
  public:
    void SetUp() override
    {
        // we only need one non default option to check whether they are correctly passed to the underlying port
        options.nodeName = "engage";

        // the default ctor is used in the getMiddlewareServer call
        PortConfigInfo portInfo;
        MemoryManager memoryManager;
        ServerPortData portData{sd, runtimeName, options, &memoryManager, portInfo.memoryInfo};
        EXPECT_CALL(*mockRuntime, getMiddlewareServer(sd, options, portInfo)).WillOnce(Return(&portData));

        sut.emplace(sd, options);
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
    iox::popo::ServerOptions options;
    optional<TestBaseServer> sut;
};

TEST_F(BaseServer_test, DestructorCallsDestroyOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "731fbc24-e4de-4223-af6b-baae6b87463d");

    EXPECT_CALL(sut->port(), destroy).Times(1);

    sut.reset(); // reset from the optional calls the dtor of the inner type
}

TEST_F(BaseServer_test, GetUidCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "3ff821b6-4977-4405-b95d-60fb84933d28");

    const UniquePortId uid;
    EXPECT_CALL(sut->port(), getUniqueID).WillOnce(Return(uid));

    EXPECT_THAT(sut->getUid(), Eq(uid));
}

TEST_F(BaseServer_test, GetServiceDescriptionCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "1481bfe5-4c70-4960-bb4c-92c637e2ec79");

    EXPECT_CALL(sut->port(), getCaProServiceDescription).WillOnce(ReturnRef(sd));

    EXPECT_THAT(sut->getServiceDescription(), Eq(sd));
}

TEST_F(BaseServer_test, OfferCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b50c652-4821-4fc2-abb4-d942db704894");

    EXPECT_CALL(sut->port(), offer).Times(1);

    sut->offer();
}

TEST_F(BaseServer_test, StopOfferCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "57e19e6e-a649-4e16-8cbc-7c7d922c0100");

    EXPECT_CALL(sut->port(), stopOffer).Times(1);

    sut->stopOffer();
}

TEST_F(BaseServer_test, IsOfferedCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "b4b46a37-5331-4306-9df1-b092ea2d62ef");

    constexpr bool IS_OFFERED{true};
    EXPECT_CALL(sut->port(), isOffered).WillOnce(Return(IS_OFFERED));

    EXPECT_THAT(sut->isOffered(), Eq(IS_OFFERED));
}

TEST_F(BaseServer_test, HasClientsCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c2eb6f3-5ce6-4bba-88de-08fcff87f5cf");

    constexpr bool HAS_CLIENTS{true};
    EXPECT_CALL(sut->port(), hasClients).WillOnce(Return(HAS_CLIENTS));

    EXPECT_THAT(sut->hasClients(), Eq(HAS_CLIENTS));
}

TEST_F(BaseServer_test, HasRequestsCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "bcc738d3-21f7-4f36-9395-4a89f2f88a07");

    constexpr bool HAS_REQUESTS{true};
    EXPECT_CALL(sut->port(), hasNewRequests).WillOnce(Return(HAS_REQUESTS));

    EXPECT_THAT(sut->hasRequests(), Eq(HAS_REQUESTS));
}

TEST_F(BaseServer_test, HasMissedRequestsCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0567e45-f66d-410b-b744-efdd4e566464");

    constexpr bool HAS_MISSED_REQUESTS{true};
    EXPECT_CALL(sut->port(), hasLostRequestsSinceLastCall).WillOnce(Return(HAS_MISSED_REQUESTS));

    EXPECT_THAT(sut->hasMissedRequests(), Eq(HAS_MISSED_REQUESTS));
}

TEST_F(BaseServer_test, ReleaseQueuedRequestsCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "3a226bd2-5c33-436a-adc6-d59e24de1074");

    EXPECT_CALL(sut->port(), releaseQueuedRequests).Times(1);

    sut->releaseQueuedRequests();
}

// BEGIN Listener and WaitSet related test

TEST_F(BaseServer_test, InvalidateTriggerWithFittingTriggerIdCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "fab216c1-b88f-4755-b5d4-7cf0fb95bc5a");

    constexpr uint64_t TRIGGER_ID{13U};

    EXPECT_CALL(sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));
    EXPECT_CALL(sut->port(), unsetConditionVariable).Times(1);
    EXPECT_CALL(sut->m_trigger, invalidate).Times(1);

    sut->invalidateTrigger(TRIGGER_ID);
}

TEST_F(BaseServer_test, InvalidateTriggerWithUnfittingTriggerIdDoesNotCallUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "a895a258-1237-4de6-ab85-7246e3404d3a");

    constexpr uint64_t TRIGGER_ID_1{1U};
    constexpr uint64_t TRIGGER_ID_2{2U};

    EXPECT_CALL(sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID_2));
    EXPECT_CALL(sut->port(), unsetConditionVariable).Times(0);
    EXPECT_CALL(sut->m_trigger, invalidate).Times(0);

    sut->invalidateTrigger(TRIGGER_ID_1);
}

TEST_F(BaseServer_test, EnableStateCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "e97beefa-f83d-42c5-8087-02bf4b9f2a32");

    for (const bool serverAttachedIndicator : {false, true})
    {
        SCOPED_TRACE(std::string("Test 'enableState' with server ")
                     + (serverAttachedIndicator ? "attached" : " not attached"));

        const uint64_t TRIGGER_ID{serverAttachedIndicator ? 42U : 73U};
        MockTriggeHandle triggerHandle;
        triggerHandle.triggerId = TRIGGER_ID;
        ConditionVariableData condVar{runtimeName};

        EXPECT_THAT(sut->m_trigger.triggerId, Ne(TRIGGER_ID));

        EXPECT_CALL(sut->m_trigger, operatorBoolMock).WillOnce(Return(serverAttachedIndicator));
        EXPECT_CALL(sut->m_trigger, getConditionVariableData).WillOnce(Return(&condVar));
        EXPECT_CALL(sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));

        EXPECT_CALL(sut->port(), setConditionVariable(Ref(condVar), TRIGGER_ID)).Times(1);

        bool errorDetected{false};
        auto errorHandlerGuard = iox::ErrorHandler::setTemporaryErrorHandler([&](const iox::Error error,
                                                                                 const std::function<void()>,
                                                                                 const iox::ErrorLevel errorLevel) {
            EXPECT_THAT(
                error,
                Eq(iox::Error::
                       kPOPO__BASE_SERVER_OVERRIDING_WITH_STATE_SINCE_HAS_REQUEST_OR_REQUEST_RECEIVED_ALREADY_ATTACHED));
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::MODERATE));
            errorDetected = true;
        });

        sut->enableState(std::move(triggerHandle), ServerState::HAS_REQUEST);

        EXPECT_THAT(sut->m_trigger.triggerId, Eq(TRIGGER_ID));
        EXPECT_THAT(errorDetected, Eq(serverAttachedIndicator));
    }
}

TEST_F(BaseServer_test, GetCallbackForIsStateConditionSatisfiedReturnsCallbackToSelf)
{
    ::testing::Test::RecordProperty("TEST_ID", "7f9d8e30-ae60-4f68-9961-ad36b4fa9bae");

    auto callback = sut->getCallbackForIsStateConditionSatisfied(ServerState::HAS_REQUEST);

    constexpr bool HAS_REQUESTS{true};
    EXPECT_CALL(sut->port(), hasNewRequests).WillOnce(Return(HAS_REQUESTS));
    EXPECT_FALSE(callback().has_error());
}

TEST_F(BaseServer_test, DisableStateCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "ce85051e-f18c-4c0f-a5c9-4c2701c4bb30");

    EXPECT_CALL(sut->m_trigger, reset).Times(1);
    EXPECT_CALL(sut->port(), unsetConditionVariable).Times(1);

    sut->disableState(ServerState::HAS_REQUEST);
}

TEST_F(BaseServer_test, EnableEventCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "a5715e02-7362-4d4e-a387-11367b804ce1");

    for (const bool serverAttachedIndicator : {false, true})
    {
        SCOPED_TRACE(std::string("Test 'enableEvent' with server ")
                     + (serverAttachedIndicator ? "attached" : " not attached"));

        const uint64_t TRIGGER_ID{serverAttachedIndicator ? 42U : 73U};
        MockTriggeHandle triggerHandle;
        triggerHandle.triggerId = TRIGGER_ID;
        ConditionVariableData condVar{runtimeName};

        EXPECT_THAT(sut->m_trigger.triggerId, Ne(TRIGGER_ID));

        EXPECT_CALL(sut->m_trigger, operatorBoolMock).WillOnce(Return(serverAttachedIndicator));
        EXPECT_CALL(sut->m_trigger, getConditionVariableData).WillOnce(Return(&condVar));
        EXPECT_CALL(sut->m_trigger, getUniqueId).WillOnce(Return(TRIGGER_ID));

        EXPECT_CALL(sut->port(), setConditionVariable(Ref(condVar), TRIGGER_ID)).Times(1);

        bool errorDetected{false};
        auto errorHandlerGuard = iox::ErrorHandler::setTemporaryErrorHandler([&](const iox::Error error,
                                                                                 const std::function<void()>,
                                                                                 const iox::ErrorLevel errorLevel) {
            EXPECT_THAT(
                error,
                Eq(iox::Error::
                       kPOPO__BASE_SERVER_OVERRIDING_WITH_EVENT_SINCE_HAS_REQUEST_OR_REQUEST_RECEIVED_ALREADY_ATTACHED));
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::MODERATE));
            errorDetected = true;
        });

        sut->enableEvent(std::move(triggerHandle), ServerEvent::REQUEST_RECEIVED);

        EXPECT_THAT(sut->m_trigger.triggerId, Eq(TRIGGER_ID));
        EXPECT_THAT(errorDetected, Eq(serverAttachedIndicator));
    }
}

TEST_F(BaseServer_test, DisableEventCallsUnderlyingPortAndTriggerHandle)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d7bee13-e654-4048-a57a-f7ba94b614b1");

    EXPECT_CALL(sut->m_trigger, reset).Times(1);
    EXPECT_CALL(sut->port(), unsetConditionVariable).Times(1);

    sut->disableEvent(ServerEvent::REQUEST_RECEIVED);
}

// END Listener and WaitSet related test

} // namespace
