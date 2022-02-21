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
#include "iceoryx_posh/internal/popo/base_client.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"
#include "mocks/client_mock.hpp"
#include "mocks/trigger_handle_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::capro;
using namespace iox::cxx;
using namespace iox::popo;
using namespace iox::runtime;
using ::testing::_;

using BaseClientWithMocks = BaseClient<MockClientPortUser, MockTriggeHandle>;

class TestBaseClient : public BaseClientWithMocks
{
  public:
    TestBaseClient(ServiceDescription sd, ClientOptions options)
        : BaseClientWithMocks::BaseClient(sd, options)
    {
    }

    using BaseClientWithMocks::port;
};

class BaseClient_test : public Test
{
  public:
    void SetUp() override
    {
        // we only need one non default option to check whether they are correctly passed to the underlying port
        options.nodeName = "engage";

        // the default ctor is used in the getMiddlewareClient call
        PortConfigInfo portInfo;

        // it is okay to not return any port since the mock does not use it anyway
        EXPECT_CALL(*mockRuntime, getMiddlewareClient(sd, options, portInfo)).Times(1);

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
    iox::popo::ClientOptions options;
    optional<TestBaseClient> sut;
};

TEST_F(BaseClient_test, DestructorCallsDestroyOnUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "fa8f6649-7889-41b1-867a-591cef414075");

    EXPECT_CALL(sut->port(), destroy).Times(1);

    sut.reset(); // reset from the optional calls the dtor of the inner type
}

TEST_F(BaseClient_test, GetUidCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c1f401c-9ee2-40f9-8f97-2ae7dae594b3");

    const UniquePortId uid;
    EXPECT_CALL(sut->port(), getUniqueID).WillOnce(Return(uid));

    EXPECT_THAT(sut->getUid(), Eq(uid));
}

TEST_F(BaseClient_test, GetServiceDescriptionCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2d46bbe-479e-4c7b-9068-7c1003584c2f");

    EXPECT_CALL(sut->port(), getCaProServiceDescription).WillOnce(ReturnRef(sd));

    EXPECT_THAT(sut->getServiceDescription(), Eq(sd));
}

TEST_F(BaseClient_test, ConnectCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "3e364583-c26b-4ba0-b55f-5121b4ed1b5f");

    EXPECT_CALL(sut->port(), connect).Times(1);

    sut->connect();
}

TEST_F(BaseClient_test, GetConnectionStateCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "f093652b-421b-43e1-b69a-6bde15f18e6d");

    constexpr ConnectionState CONNECTION_STATE{ConnectionState::WAIT_FOR_OFFER};
    EXPECT_CALL(sut->port(), getConnectionState).WillOnce(Return(CONNECTION_STATE));

    EXPECT_THAT(sut->getConnectionState(), Eq(CONNECTION_STATE));
}

TEST_F(BaseClient_test, DisconnectCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "025b478a-c9b7-4f08-821f-f3f4abdc6f65");

    EXPECT_CALL(sut->port(), disconnect).Times(1);

    sut->disconnect();
}

TEST_F(BaseClient_test, HasResponsesCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "8d50f56a-a489-4c5c-9d17-c966fb7e171c");

    constexpr bool HAS_RESPONSES{true};
    EXPECT_CALL(sut->port(), hasNewResponses).WillOnce(Return(HAS_RESPONSES));

    EXPECT_THAT(sut->hasResponses(), Eq(HAS_RESPONSES));
}

TEST_F(BaseClient_test, HasMissedResponsesCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "0a0a8bf6-47af-4ce4-acbb-adf7c09513f6");

    constexpr bool HAS_MISSED_RESPONSES{true};
    EXPECT_CALL(sut->port(), hasLostResponsesSinceLastCall).WillOnce(Return(HAS_MISSED_RESPONSES));

    EXPECT_THAT(sut->hasMissedResponses(), Eq(HAS_MISSED_RESPONSES));
}

TEST_F(BaseClient_test, ReleaseQueuedResponsesCallsUnderlyingPort)
{
    ::testing::Test::RecordProperty("TEST_ID", "bd72358c-dc0c-4900-bea5-52be800f1448");

    EXPECT_CALL(sut->port(), releaseQueuedResponses).Times(1);

    sut->releaseQueuedResponses();
}

} // namespace
