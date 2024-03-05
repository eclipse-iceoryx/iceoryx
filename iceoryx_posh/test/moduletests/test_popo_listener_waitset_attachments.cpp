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

#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/server.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/popo/untyped_server.hpp"
#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"

#include "test.hpp"
namespace
{
using namespace ::testing;
using namespace ::iox::popo;
using namespace ::iox;
using namespace ::iox::capro;
using namespace ::iox::mepoo;

constexpr const char RUNTIME_NAME[] = "torben_dallas";
constexpr const char SERVICE[] = "respect_to_the_man_in_the_icecream_van";
constexpr const char INSTANCE[] = "Lakierski materialski";
constexpr const char EVENT[] = "boom boom boomerang";

/// Those tests verify that the destructor of the attachables is correct. When we
/// use inheritance it is possible that the trigger is a member of the base class
/// and destroyed in the base class constructor as well.
/// But the listener and waitset require the child class for cleanup. If the child
/// class destructor does not call reset on all the triggers the base class will
/// and the listener and waitset will call a method on the original class which is
/// already deleted. This causes undefined behavior which the sanitizer will catch.
///
/// The following tests should be run for all classes which can be attached to a
/// listener or a waitset to ensure that trigger.reset() is called in the child destructor
/// when we work with inheritance.
///
/// When no inheritance is used we shouldn't encounter any problems.
///
/// It suffices to call all tests on the listener since the listener and the waitset
/// are using the same trigger concept
///
/// Strategy:
///   We attach every attachable to the listener and call the destructor while it is attached.
///   When those classes use inheritance and the underlying trigger is called in the base
///   class the address sanitizer will encounter undefined behavior.
class ListenerWaitsetAttachments_test : public Test
{
  public:
    void SetUp()
    {
        EXPECT_CALL(*this->runtimeMock, getMiddlewareConditionVariable())
            .WillOnce(Return(&this->conditionVariableData));
        listener.emplace();
    }

    template <typename T>
    static void genericTriggerCallback(T* const)
    {
    }

    std::unique_ptr<PoshRuntimeMock> runtimeMock = PoshRuntimeMock::create(RUNTIME_NAME);
    ConditionVariableData conditionVariableData{RUNTIME_NAME};
    optional<Listener> listener;

    MemoryManager memoryManager;
};


TEST_F(ListenerWaitsetAttachments_test, SubscriberDestructorCallsTriggerResetDirectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "f1d163e0-4479-47b4-b4e8-01b0ee6a71b0");
    SubscriberPortData subscriberData({SERVICE, INSTANCE, EVENT},
                                      RUNTIME_NAME,
                                      roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                      VariantQueueTypes::SoFi_MultiProducerSingleConsumer,
                                      SubscriberOptions());
    EXPECT_CALL(*this->runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&subscriberData));

    optional<Subscriber<int>> subscriber;
    subscriber.emplace(ServiceDescription(SERVICE, INSTANCE, EVENT));

    ASSERT_FALSE(listener
                     ->attachEvent(*subscriber,
                                   SubscriberEvent::DATA_RECEIVED,
                                   createNotificationCallback(
                                       ListenerWaitsetAttachments_test::genericTriggerCallback<Subscriber<int>>))
                     .has_error());

    EXPECT_THAT(listener->size(), Eq(1));
    subscriber.reset();
    EXPECT_THAT(listener->size(), Eq(0));
}

TEST_F(ListenerWaitsetAttachments_test, UntypedSubscriberDestructorCallsTriggerResetDirectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "a78a7016-46b6-4223-b7b1-e30344bb208f");
    SubscriberPortData subscriberData({SERVICE, INSTANCE, EVENT},
                                      RUNTIME_NAME,
                                      roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                      VariantQueueTypes::SoFi_MultiProducerSingleConsumer,
                                      SubscriberOptions());
    EXPECT_CALL(*this->runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&subscriberData));

    optional<UntypedSubscriber> subscriber;
    subscriber.emplace(ServiceDescription(SERVICE, INSTANCE, EVENT));

    ASSERT_FALSE(listener
                     ->attachEvent(*subscriber,
                                   SubscriberEvent::DATA_RECEIVED,
                                   createNotificationCallback(
                                       ListenerWaitsetAttachments_test::genericTriggerCallback<UntypedSubscriber>))
                     .has_error());

    EXPECT_THAT(listener->size(), Eq(1));
    subscriber.reset();
    EXPECT_THAT(listener->size(), Eq(0));
}

TEST_F(ListenerWaitsetAttachments_test, ClientDestructorCallsTriggerResetDirectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "e21d98b9-9d24-4c85-90b9-0e7acd24a242");
    ClientPortData clientData(
        {SERVICE, INSTANCE, EVENT}, RUNTIME_NAME, roudi::DEFAULT_UNIQUE_ROUDI_ID, ClientOptions(), &memoryManager);
    EXPECT_CALL(*this->runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientData));

    optional<Client<int, int>> client;
    client.emplace(ServiceDescription(SERVICE, INSTANCE, EVENT));

    ASSERT_FALSE(listener
                     ->attachEvent(*client,
                                   ClientEvent::RESPONSE_RECEIVED,
                                   createNotificationCallback(
                                       ListenerWaitsetAttachments_test::genericTriggerCallback<Client<int, int>>))
                     .has_error());

    EXPECT_THAT(listener->size(), Eq(1));
    client.reset();
    EXPECT_THAT(listener->size(), Eq(0));
}

TEST_F(ListenerWaitsetAttachments_test, UntypedClientDestructorCallsTriggerResetDirectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "07934dd2-93aa-4aab-a216-eb86e842088b");
    ClientPortData clientData(
        {SERVICE, INSTANCE, EVENT}, RUNTIME_NAME, roudi::DEFAULT_UNIQUE_ROUDI_ID, ClientOptions(), &memoryManager);
    EXPECT_CALL(*this->runtimeMock, getMiddlewareClient(_, _, _)).WillOnce(Return(&clientData));

    optional<UntypedClient> client;
    client.emplace(ServiceDescription(SERVICE, INSTANCE, EVENT));

    ASSERT_FALSE(listener
                     ->attachEvent(*client,
                                   ClientEvent::RESPONSE_RECEIVED,
                                   createNotificationCallback(
                                       ListenerWaitsetAttachments_test::genericTriggerCallback<UntypedClient>))
                     .has_error());

    EXPECT_THAT(listener->size(), Eq(1));
    client.reset();
    EXPECT_THAT(listener->size(), Eq(0));
}

TEST_F(ListenerWaitsetAttachments_test, ServerDestructorCallsTriggerResetDirectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "306d7ef9-1fb1-4ce8-8b58-e5a7cb5fff69");
    ServerPortData serverData(
        {SERVICE, INSTANCE, EVENT}, RUNTIME_NAME, roudi::DEFAULT_UNIQUE_ROUDI_ID, ServerOptions(), &memoryManager);
    EXPECT_CALL(*this->runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverData));

    optional<Server<int, int>> server;
    server.emplace(ServiceDescription(SERVICE, INSTANCE, EVENT));

    ASSERT_FALSE(listener
                     ->attachEvent(*server,
                                   ServerEvent::REQUEST_RECEIVED,
                                   createNotificationCallback(
                                       ListenerWaitsetAttachments_test::genericTriggerCallback<Server<int, int>>))
                     .has_error());

    EXPECT_THAT(listener->size(), Eq(1));
    server.reset();
    EXPECT_THAT(listener->size(), Eq(0));
}

TEST_F(ListenerWaitsetAttachments_test, UntypedServerDestructorCallsTriggerResetDirectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "3d074e8f-eab6-475d-a884-de8b4d2596ea");
    ServerPortData serverData(
        {SERVICE, INSTANCE, EVENT}, RUNTIME_NAME, roudi::DEFAULT_UNIQUE_ROUDI_ID, ServerOptions(), &memoryManager);
    EXPECT_CALL(*this->runtimeMock, getMiddlewareServer(_, _, _)).WillOnce(Return(&serverData));

    optional<UntypedServer> server;
    server.emplace(ServiceDescription(SERVICE, INSTANCE, EVENT));

    ASSERT_FALSE(listener
                     ->attachEvent(*server,
                                   ServerEvent::REQUEST_RECEIVED,
                                   createNotificationCallback(
                                       ListenerWaitsetAttachments_test::genericTriggerCallback<UntypedServer>))
                     .has_error());

    EXPECT_THAT(listener->size(), Eq(1));
    server.reset();
    EXPECT_THAT(listener->size(), Eq(0));
}

TEST_F(ListenerWaitsetAttachments_test, ServiceDiscoveryDestructorCallsTriggerResetDirectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "b266bb98-f31a-43b8-a0c4-75aea6f40efb");
    SubscriberPortData subscriberData({SERVICE, INSTANCE, EVENT},
                                      RUNTIME_NAME,
                                      roudi::DEFAULT_UNIQUE_ROUDI_ID,
                                      VariantQueueTypes::SoFi_MultiProducerSingleConsumer,
                                      SubscriberOptions());
    EXPECT_CALL(*this->runtimeMock, getMiddlewareSubscriber(_, _, _)).WillOnce(Return(&subscriberData));

    optional<iox::runtime::ServiceDiscovery> serviceDiscovery;
    serviceDiscovery.emplace();

    ASSERT_FALSE(
        listener
            ->attachEvent(*serviceDiscovery,
                          iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED,
                          createNotificationCallback(
                              ListenerWaitsetAttachments_test::genericTriggerCallback<iox::runtime::ServiceDiscovery>))
            .has_error());

    EXPECT_THAT(listener->size(), Eq(1));
    serviceDiscovery.reset();
    EXPECT_THAT(listener->size(), Eq(0));
}

} // namespace
