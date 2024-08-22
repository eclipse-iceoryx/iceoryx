// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2022 by NXP. All rights reserved.
// Copyright (c) 2024 by Bartlomiej Kozaryna <kozarynabartlomiej@gmail.com>. All rights reserved.
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

#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/roudi_env/minimal_iceoryx_config.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"
#include "iox/atomic.hpp"
#include "iox/forward_list.hpp"
#include "iox/list.hpp"
#include "iox/optional.hpp"
#include "iox/stack.hpp"
#include "iox/string.hpp"
#include "iox/variant.hpp"
#include "iox/vector.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox;
using namespace iox::popo;
using namespace iox::roudi_env;
using namespace iox::testing;

constexpr uint64_t SIZE_LARGER_THAN_4GB = std::numeric_limits<uint32_t>::max() + 41065UL;

template <typename T>
struct ComplexDataType
{
    int64_t someNumber = 0;
    T complexType;
};

struct BigPayloadStruct
{
    uint8_t bigPayload[SIZE_LARGER_THAN_4GB];
};

class PublisherSubscriberCommunication_test : public RouDi_GTest
{
  protected:
    PublisherSubscriberCommunication_test(iox::IceoryxConfig&& config)
        : RouDi_GTest(std::move(config))
    {
    }

  public:
    PublisherSubscriberCommunication_test()
        : RouDi_GTest(MinimalIceoryxConfigBuilder().payloadChunkSize(512).create())
    {
    }

    void SetUp() override
    {
        runtime::PoshRuntime::initRuntime("PublisherSubscriberCommunication_test");
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    };
    void TearDown() override
    {
    }

    template <typename T>
    std::unique_ptr<iox::popo::Publisher<T>>
    createPublisher(const ConsumerTooSlowPolicy policy = ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA,
                    const capro::Interfaces interface = capro::Interfaces::INTERNAL)
    {
        iox::popo::PublisherOptions options;
        options.subscriberTooSlowPolicy = policy;
        return std::make_unique<iox::popo::Publisher<T>>(
            capro::ServiceDescription{m_serviceDescription.getServiceIDString(),
                                      m_serviceDescription.getInstanceIDString(),
                                      m_serviceDescription.getEventIDString(),
                                      {0U, 0U, 0U, 0U},
                                      interface},
            options);
    }

    // additional overload for convenient use in the test cases
    template <typename T>
    std::unique_ptr<iox::popo::Publisher<T>> createPublisher(const uint64_t historyCapacity)
    {
        iox::popo::PublisherOptions options;
        options.historyCapacity = historyCapacity;
        return std::make_unique<iox::popo::Publisher<T>>(
            capro::ServiceDescription{m_serviceDescription.getServiceIDString(),
                                      m_serviceDescription.getInstanceIDString(),
                                      m_serviceDescription.getEventIDString(),
                                      {0U, 0U, 0U, 0U},
                                      capro::Interfaces::INTERNAL},
            options);
    }

    template <typename T>
    std::unique_ptr<iox::popo::Subscriber<T>>
    createSubscriber(const QueueFullPolicy policy = QueueFullPolicy::DISCARD_OLDEST_DATA,
                     const uint64_t queueCapacity = SubscriberPortData::ChunkQueueData_t::MAX_CAPACITY,
                     const capro::Interfaces interface = capro::Interfaces::INTERNAL)
    {
        iox::popo::SubscriberOptions options;
        options.queueFullPolicy = policy;
        options.queueCapacity = queueCapacity;
        return std::make_unique<iox::popo::Subscriber<T>>(
            capro::ServiceDescription{m_serviceDescription.getServiceIDString(),
                                      m_serviceDescription.getInstanceIDString(),
                                      m_serviceDescription.getEventIDString(),
                                      {0U, 0U, 0U, 0U},
                                      interface},
            options);
    }

    // additional overload for convenient use in the test cases
    template <typename T>
    std::unique_ptr<iox::popo::Subscriber<T>> createSubscriber(const uint64_t historyRequest,
                                                               const bool requiresPublisherHistorySupport = false)
    {
        iox::popo::SubscriberOptions options;
        options.historyRequest = historyRequest;
        options.requiresPublisherHistorySupport = requiresPublisherHistorySupport;
        return std::make_unique<iox::popo::Subscriber<T>>(
            capro::ServiceDescription{m_serviceDescription.getServiceIDString(),
                                      m_serviceDescription.getInstanceIDString(),
                                      m_serviceDescription.getEventIDString(),
                                      {0U, 0U, 0U, 0U},
                                      capro::Interfaces::INTERNAL},
            options);
    }

    Watchdog m_watchdog{units::Duration::fromSeconds(5)};
    capro::ServiceDescription m_serviceDescription{
        "PublisherSubscriberCommunication", "IntegrationTest", "AllHailHypnotoad"};
};

class PublisherSubscriberCommunicationWithBigPayload_test : public PublisherSubscriberCommunication_test
{
    static constexpr uint64_t additionalSizeForUserHeader =
        2 * std::max(sizeof(iox::popo::RequestHeader), sizeof(iox::popo::ResponseHeader));

  public:
    PublisherSubscriberCommunicationWithBigPayload_test()
        : PublisherSubscriberCommunication_test(
            MinimalIceoryxConfigBuilder()
                .payloadChunkSize(SIZE_LARGER_THAN_4GB + additionalSizeForUserHeader)
                .payloadChunkCount(2)
                .create())
    {
    }

    void SetUp()
    {
        runtime::PoshRuntime::initRuntime("PublisherSubscriberCommunication_test");
        m_watchdog.watchAndActOnFailure([] { std::terminate(); });
    };
    Watchdog m_watchdog{units::Duration::fromSeconds(10)};
};

// intentional reference to unique pointer, we do not want to pass ownership in this helper function
// and at call site we already have unique pointers
template <typename T>
void publishAndExpectReceivedData(const std::unique_ptr<iox::popo::Publisher<T>>& pub,
                                  const std::unique_ptr<iox::popo::Subscriber<T>>& sub,
                                  T data)
{
    ASSERT_FALSE(pub->loan()
                     .and_then([&](auto& sample) {
                         *sample = data;
                         sample.publish();
                     })
                     .has_error());

    auto result = sub->take();
    ASSERT_FALSE(result.has_error());
    EXPECT_EQ(*result.value().get(), data);
}

TEST_F(PublisherSubscriberCommunication_test, AllSubscriberInterfacesCanBeSubscribedToPublisherWithInternalInterface)
{
    ::testing::Test::RecordProperty("TEST_ID", "aba18b27-bf64-49a7-8ad6-06a84b23a455");
    auto publisher = createPublisher<int>();

    std::vector<std::unique_ptr<iox::popo::Subscriber<int>>> subscribers;
    for (uint16_t interface = 0U; interface < static_cast<uint16_t>(capro::Interfaces::INTERFACE_END); ++interface)
    {
        subscribers.emplace_back(createSubscriber<int>(QueueFullPolicy::DISCARD_OLDEST_DATA,
                                                       SubscriberPortData::ChunkQueueData_t::MAX_CAPACITY,
                                                       static_cast<capro::Interfaces>(interface)));
    }

    constexpr int TRANSMISSION_DATA = 1337;
    ASSERT_FALSE(publisher->loan()
                     .and_then([&](auto& sample) {
                         *sample = TRANSMISSION_DATA;
                         sample.publish();
                     })
                     .has_error());

    for (auto& subscriber : subscribers)
    {
        EXPECT_FALSE(subscriber->take()
                         .and_then([&](auto& sample) { EXPECT_THAT(*sample, Eq(TRANSMISSION_DATA)); })
                         .has_error());
    }
}

TEST_F(PublisherSubscriberCommunication_test,
       SubscriberRequiringHistorySupportDoesNotConnectToPublisherWithoutHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "31cbd36d-32f1-4bc7-9980-0cdf5f248035");

    constexpr uint64_t historyRequest = 1;
    constexpr uint64_t historyCapacity = 0;
    constexpr bool requiresHistorySupport = true;

    auto publisher = createPublisher<int>(historyCapacity);
    auto subscriber = createSubscriber<int>(historyRequest, requiresHistorySupport);

    ASSERT_TRUE(publisher);
    EXPECT_FALSE(publisher->hasSubscribers());
}

TEST_F(PublisherSubscriberCommunication_test,
       SubscriberNotRequiringHistorySupportDoesConnectToPublisherWithNoHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "c47f5ebd-044c-480b-a4bb-d700655105ac");

    constexpr uint64_t historyRequest = 1;
    constexpr uint64_t historyCapacity = 0;
    constexpr bool requiresHistorySupport = false;

    auto publisher = createPublisher<int>(historyCapacity);
    auto subscriber = createSubscriber<int>(historyRequest, requiresHistorySupport);

    ASSERT_TRUE(publisher);
    EXPECT_TRUE(publisher->hasSubscribers());

    publishAndExpectReceivedData(publisher, subscriber, 73);
}

TEST_F(PublisherSubscriberCommunication_test,
       SubscriberRequiringHistorySupportDoesConnectToPublisherWithEqualHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ca391fe-c4f6-48b5-bd36-96854513c6bb");

    constexpr uint64_t historyRequest = 3;
    constexpr uint64_t historyCapacity = 3;
    constexpr bool requiresHistorySupport = true;

    auto publisher = createPublisher<int>(historyCapacity);
    auto subscriber = createSubscriber<int>(historyRequest, requiresHistorySupport);

    ASSERT_TRUE(publisher);
    EXPECT_TRUE(publisher->hasSubscribers());

    publishAndExpectReceivedData(publisher, subscriber, 74);
}

TEST_F(PublisherSubscriberCommunication_test,
       SubscriberRequiringHistorySupportDoesConnectToPublisherWithLowerHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "46b917e6-75f1-4cd2-8ffa-1c254f3423a7");

    constexpr uint64_t historyRequest = 6;
    constexpr uint64_t historyCapacity = 5;
    constexpr bool requiresHistorySupport = true;

    auto publisher = createPublisher<int>(historyCapacity);
    auto subscriber = createSubscriber<int>(historyRequest, requiresHistorySupport);

    ASSERT_TRUE(publisher);
    EXPECT_TRUE(publisher->hasSubscribers());

    publishAndExpectReceivedData(publisher, subscriber, 75);
}

TEST_F(PublisherSubscriberCommunication_test,
       SubscriberNotRequiringHistorySupportDoesConnectToPublisherWithLowerHistorySupport)
{
    ::testing::Test::RecordProperty("TEST_ID", "b672c382-f81b-4cd4-8049-36d2691bb532");

    constexpr uint64_t historyRequest = 3;
    constexpr uint64_t historyCapacity = 2;
    constexpr bool requiresHistorySupport = false;

    auto publisher = createPublisher<int>(historyCapacity);
    auto subscriber = createSubscriber<int>(historyRequest, requiresHistorySupport);

    ASSERT_TRUE(publisher);
    EXPECT_TRUE(publisher->hasSubscribers());

    publishAndExpectReceivedData(publisher, subscriber, 76);
}

TEST_F(PublisherSubscriberCommunication_test, SubscriberCanOnlyBeSubscribedWhenInterfaceDiffersFromPublisher)
{
    ::testing::Test::RecordProperty("TEST_ID", "c01fa002-84ae-4017-a801-e790a3a04702");
    for (uint16_t publisherInterface = 0U; publisherInterface < static_cast<uint16_t>(capro::Interfaces::INTERFACE_END);
         ++publisherInterface)
    {
        if (static_cast<capro::Interfaces>(publisherInterface) == capro::Interfaces::INTERNAL)
        {
            continue;
        }

        m_watchdog.watchAndActOnFailure();

        auto publisher = createPublisher<int>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA,
                                              static_cast<capro::Interfaces>(publisherInterface));

        std::vector<std::unique_ptr<iox::popo::Subscriber<int>>> subscribers;
        for (uint16_t subscriberInterface = 0U;
             subscriberInterface < static_cast<uint16_t>(capro::Interfaces::INTERFACE_END);
             ++subscriberInterface)
        {
            subscribers.emplace_back(createSubscriber<int>(QueueFullPolicy::DISCARD_OLDEST_DATA,
                                                           SubscriberPortData::ChunkQueueData_t::MAX_CAPACITY,
                                                           static_cast<capro::Interfaces>(subscriberInterface)));
        }

        constexpr int TRANSMISSION_DATA = 1337;
        ASSERT_FALSE(publisher->loan()
                         .and_then([&](auto& sample) {
                             *sample = TRANSMISSION_DATA;
                             sample.publish();
                         })
                         .has_error());

        for (auto& subscriber : subscribers)
        {
            if (subscriber->getServiceDescription().getSourceInterface()
                == static_cast<capro::Interfaces>(publisherInterface))
            {
                EXPECT_TRUE(subscriber->take().has_error());
            }
            else
            {
                EXPECT_FALSE(subscriber->take()
                                 .and_then([&](auto& sample) { EXPECT_THAT(*sample, Eq(TRANSMISSION_DATA)); })
                                 .has_error());
            }
        }
    }
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_forward_list)
{
    ::testing::Test::RecordProperty("TEST_ID", "97cbebbe-d430-4437-881d-90329e73dd42");
    using Type_t = ComplexDataType<forward_list<string<5>, 5>>;
    auto publisher = createPublisher<Type_t>();
    auto subscriber = createSubscriber<Type_t>();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 123;
                         sample->complexType.push_front("world");
                         sample->complexType.push_front("hello");
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(123));
                         ASSERT_THAT(sample->complexType.size(), Eq(2U));
                         auto begin = sample->complexType.begin();
                         EXPECT_THAT(*begin, Eq(string<5>("hello")));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(string<5>("world")));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_list)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c5fa83a-935d-46ba-8adf-91e1de6acc89");
    using Type_t = ComplexDataType<list<int64_t, 5>>;
    auto publisher = createPublisher<Type_t>();
    auto subscriber = createSubscriber<Type_t>();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 4123;
                         sample->complexType.push_front(77);
                         sample->complexType.push_front(66);
                         sample->complexType.push_front(55);
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(4123));
                         ASSERT_THAT(sample->complexType.size(), Eq(3U));
                         auto begin = sample->complexType.begin();
                         EXPECT_THAT(*begin, Eq(55));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(66));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(77));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_optional)
{
    ::testing::Test::RecordProperty("TEST_ID", "341ff552-a7a7-4dd9-be83-29d41bf142ec");
    using Type_t = ComplexDataType<list<optional<int32_t>, 5>>;
    auto publisher = createPublisher<Type_t>();
    auto subscriber = createSubscriber<Type_t>();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 41231;
                         sample->complexType.push_front(177);
                         sample->complexType.push_front(nullopt);
                         sample->complexType.push_front(155);
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(41231));
                         ASSERT_THAT(sample->complexType.size(), Eq(3U));
                         auto begin = sample->complexType.begin();
                         EXPECT_THAT(*begin, Eq(optional<int32_t>(155)));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(nullopt));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(optional<int32_t>(177)));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_stack)
{
    ::testing::Test::RecordProperty("TEST_ID", "c378e0db-d863-4cad-9efa-4daec364b266");
    using Type_t = ComplexDataType<stack<uint64_t, 10>>;
    auto publisher = createPublisher<Type_t>();
    auto subscriber = createSubscriber<Type_t>();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 41231;
                         for (uint64_t i = 0U; i < 10U; ++i)
                         {
                             sample->complexType.push(i + 123U);
                         }
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(41231));
                         ASSERT_THAT(sample->complexType.size(), Eq(10U));
                         auto stackCopy = sample->complexType;
                         for (uint64_t i = 0U; i < 10U; ++i)
                         {
                             auto result = stackCopy.pop();
                             ASSERT_THAT(result.has_value(), Eq(true));
                             EXPECT_THAT(*result, Eq(123 + 9 - i));
                         }
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_string)
{
    ::testing::Test::RecordProperty("TEST_ID", "0603b4ca-f41a-4280-9984-cf1465ee05c7");
    using Type_t = ComplexDataType<string<128>>;
    auto publisher = createPublisher<Type_t>();
    auto subscriber = createSubscriber<Type_t>();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 123;
                         sample->complexType = "You're my Heart, You're my Seal!";
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(123));
                         EXPECT_THAT(sample->complexType, Eq(string<128>("You're my Heart, You're my Seal!")));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_vector)
{
    ::testing::Test::RecordProperty("TEST_ID", "fdfe4d05-c61a-4a99-b0b7-5e79da2700d5");
    using Type_t = ComplexDataType<vector<string<64>, 5>>;
    auto publisher = createPublisher<Type_t>();
    auto subscriber = createSubscriber<Type_t>();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 123;
                         sample->complexType.emplace_back("Don't stop the hypnotoad");
                         sample->complexType.emplace_back("Be like hypnotoad");
                         sample->complexType.emplace_back("Piep, piep little satellite");
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(123));
                         ASSERT_THAT(sample->complexType.size(), Eq(3));
                         EXPECT_THAT(sample->complexType[0], Eq(string<128>("Don't stop the hypnotoad")));
                         EXPECT_THAT(sample->complexType[1], Eq(string<128>("Be like hypnotoad")));
                         EXPECT_THAT(sample->complexType[2], Eq(string<128>("Piep, piep little satellite")));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_variant)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b5688ff-2367-4c76-93a2-6e447403c5ed");
    using Type_t = ComplexDataType<vector<variant<string<64>, int>, 5>>;
    auto publisher = createPublisher<Type_t>();
    auto subscriber = createSubscriber<Type_t>();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 123;
                         sample->complexType.emplace_back(in_place_index<0>(), "Be aware! Bob is a vampire!");
                         sample->complexType.emplace_back(in_place_index<1>(), 1337);
                         sample->complexType.emplace_back(in_place_index<0>(), "Bob is an acronym for Bob Only Bob");
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(123));
                         ASSERT_THAT(sample->complexType.size(), Eq(3));
                         ASSERT_THAT(sample->complexType[0].index(), Eq(0));
                         EXPECT_THAT(*sample->complexType[0].template get_at_index<0>(),
                                     Eq(string<128>("Be aware! Bob is a vampire!")));
                         ASSERT_THAT(sample->complexType[1].index(), Eq(1));
                         EXPECT_THAT(*sample->complexType[1].template get_at_index<1>(), Eq(1337));
                         ASSERT_THAT(sample->complexType[2].index(), Eq(0));
                         EXPECT_THAT(*sample->complexType[2].template get_at_index<0>(),
                                     Eq(string<128>("Bob is an acronym for Bob Only Bob")));
                     })
                     .has_error());
}


TEST_F(PublisherSubscriberCommunication_test, PublisherBlocksWhenBlockingActivatedOnBothSidesAndSubscriberQueueIsFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "e97f1665-3488-4288-8fde-f485067bfeb4");
    auto publisher = createPublisher<string<128>>(ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER);

    auto subscriber = createSubscriber<string<128>>(QueueFullPolicy::BLOCK_PRODUCER, 2U);

    EXPECT_FALSE(publisher->publishCopyOf("start your day with a smile").has_error());
    EXPECT_FALSE(publisher->publishCopyOf("and hypnotoad will smile back").has_error());

    iox::concurrent::Atomic<bool> wasSampleDelivered{false};
    Barrier isThreadStarted(1U);
    std::thread t1([&] {
        isThreadStarted.notify();
        EXPECT_FALSE(publisher->publishCopyOf("oh no hypnotoad is staring at me").has_error());
        wasSampleDelivered.store(true);
    });

    constexpr int64_t TIMEOUT_IN_MS = 100;

    isThreadStarted.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_IN_MS));
    EXPECT_FALSE(wasSampleDelivered.load());

    auto sample = subscriber->take();
    ASSERT_FALSE(sample.has_error());
    EXPECT_THAT(**sample, Eq(string<128>("start your day with a smile")));
    t1.join(); // join needs to be before the load to ensure the wasSampleDelivered store happens before the
               // read
    EXPECT_TRUE(wasSampleDelivered.load());

    EXPECT_FALSE(subscriber->hasMissedData());
    sample = subscriber->take();
    ASSERT_FALSE(sample.has_error());
    EXPECT_THAT(**sample, Eq(string<128>("and hypnotoad will smile back")));

    sample = subscriber->take();
    ASSERT_FALSE(sample.has_error());
    EXPECT_THAT(**sample, Eq(string<128>("oh no hypnotoad is staring at me")));
}

TEST_F(PublisherSubscriberCommunication_test, PublisherDoesNotBlockAndDiscardsSamplesWhenNonBlockingActivated)
{
    ::testing::Test::RecordProperty("TEST_ID", "1d92226d-fb3a-487c-bf52-6eb3c7946dc6");
    auto publisher = createPublisher<string<128>>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA);

    auto subscriber = createSubscriber<string<128>>(QueueFullPolicy::DISCARD_OLDEST_DATA, 2U);

    EXPECT_FALSE(publisher->publishCopyOf("first there was a blubb named mantua").has_error());
    EXPECT_FALSE(publisher->publishCopyOf("second hypnotoad ate it").has_error());

    iox::concurrent::Atomic<bool> wasSampleDelivered{false};
    Barrier isThreadStarted(1U);
    std::thread t1([&] {
        isThreadStarted.notify();
        EXPECT_FALSE(publisher->publishCopyOf("third a tiny black hole smells like butter").has_error());
        wasSampleDelivered.store(true);
    });

    isThreadStarted.wait();
    t1.join();
    EXPECT_TRUE(wasSampleDelivered.load());

    EXPECT_TRUE(subscriber->hasMissedData());
    auto sample = subscriber->take();
    ASSERT_FALSE(sample.has_error());
    EXPECT_THAT(**sample, Eq(string<128>("second hypnotoad ate it")));

    sample = subscriber->take();
    ASSERT_FALSE(sample.has_error());
    EXPECT_THAT(**sample, Eq(string<128>("third a tiny black hole smells like butter")));
}

TEST_F(PublisherSubscriberCommunication_test, NoSubscriptionWhenSubscriberWantsBlockingAndPublisherDoesNotOfferBlocking)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0144704-6dd7-4354-a41d-d4e512633484");
    auto publisher = createPublisher<string<128>>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA);

    auto subscriber = createSubscriber<string<128>>(QueueFullPolicy::BLOCK_PRODUCER, 2U);

    EXPECT_FALSE(publisher->publishCopyOf("never kiss the hypnotoad").has_error());

    auto sample = subscriber->take();
    EXPECT_THAT(sample.has_error(), Eq(true));
}

TEST_F(PublisherSubscriberCommunication_test, SubscriptionWhenSubscriberDoesNotRequireBlockingButPublisherSupportsIt)
{
    ::testing::Test::RecordProperty("TEST_ID", "228ea848-8926-4779-9e38-4d92eeb87feb");
    auto publisher = createPublisher<string<128>>(ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER);

    auto subscriber = createSubscriber<string<128>>(QueueFullPolicy::DISCARD_OLDEST_DATA, 2U);

    EXPECT_FALSE(publisher->publishCopyOf("never kiss the hypnotoad").has_error());

    auto sample = subscriber->take();
    EXPECT_THAT(sample.has_error(), Eq(false));
    EXPECT_THAT(**sample, Eq(string<128>("never kiss the hypnotoad")));
}

TEST_F(PublisherSubscriberCommunication_test, MixedOptionsSetupWorksWithBlocking)
{
    ::testing::Test::RecordProperty("TEST_ID", "c60ade45-1765-40ca-bc4b-7452c82ba127");
    auto publisherBlocking = createPublisher<string<128>>(ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER);
    auto publisherNonBlocking = createPublisher<string<128>>(ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA);

    auto subscriberBlocking = createSubscriber<string<128>>(QueueFullPolicy::BLOCK_PRODUCER, 2U);
    auto subscriberNonBlocking = createSubscriber<string<128>>(QueueFullPolicy::DISCARD_OLDEST_DATA, 2U);

    EXPECT_FALSE(publisherBlocking->publishCopyOf("hypnotoads real name is Salsabarh Slimekirkdingle").has_error());
    EXPECT_FALSE(publisherBlocking->publishCopyOf("hypnotoad wants a cookie").has_error());
    EXPECT_FALSE(publisherNonBlocking->publishCopyOf("hypnotoad has a sister named hypnoodle").has_error());

    iox::concurrent::Atomic<bool> wasSampleDelivered{false};
    Barrier isThreadStarted(1U);
    std::thread t1([&] {
        isThreadStarted.notify();
        EXPECT_FALSE(publisherBlocking->publishCopyOf("chucky is the only one who can ride the hypnotoad").has_error());
        wasSampleDelivered.store(true);
    });

    constexpr int64_t TIMEOUT_IN_MS = 100;

    isThreadStarted.wait();
    std::this_thread::sleep_for(std::chrono::milliseconds(TIMEOUT_IN_MS));
    EXPECT_FALSE(wasSampleDelivered.load());

    // verify blocking subscriber
    auto sample = subscriberBlocking->take();
    EXPECT_THAT(sample.has_error(), Eq(false));
    EXPECT_THAT(**sample, Eq(string<128>("hypnotoads real name is Salsabarh Slimekirkdingle")));
    t1.join(); // join needs to be before the load to ensure the wasSampleDelivered store happens before the
               // read
    EXPECT_TRUE(wasSampleDelivered.load());

    EXPECT_FALSE(subscriberBlocking->hasMissedData()); // we don't loose samples here
    sample = subscriberBlocking->take();
    EXPECT_THAT(sample.has_error(), Eq(false));
    EXPECT_THAT(**sample, Eq(string<128>("hypnotoad wants a cookie")));

    sample = subscriberBlocking->take();
    EXPECT_THAT(sample.has_error(), Eq(false));
    EXPECT_THAT(**sample, Eq(string<128>("chucky is the only one who can ride the hypnotoad")));
    EXPECT_THAT(subscriberBlocking->take().has_error(), Eq(true));

    // verify non blocking subscriber
    EXPECT_TRUE(subscriberNonBlocking->hasMissedData()); // we do loose samples here
    sample = subscriberNonBlocking->take();
    EXPECT_THAT(sample.has_error(), Eq(false));
    EXPECT_THAT(**sample, Eq(string<128>("hypnotoad has a sister named hypnoodle")));

    sample = subscriberNonBlocking->take();
    EXPECT_THAT(sample.has_error(), Eq(false));
    EXPECT_THAT(**sample, Eq(string<128>("chucky is the only one who can ride the hypnotoad")));
    EXPECT_THAT(subscriberNonBlocking->take().has_error(), Eq(true));
}

TEST_F(PublisherSubscriberCommunication_test, PublisherUniqueIdMatchesReceivedSample)
{
    ::testing::Test::RecordProperty("TEST_ID", "decbfcdd-778f-4e18-b6a8-395d400fdd80");

    auto publisher = createPublisher<int>();
    auto subscriber = createSubscriber<int>();

    const auto uid = publisher->getUid();

    for (int i = 0; i < 10; ++i)
    {
        ASSERT_FALSE(publisher->loan()
                         .and_then([&](auto& sample) {
                             *sample = i;
                             sample.publish();
                         })
                         .has_error());

        EXPECT_FALSE(subscriber->take()
                         .and_then([&](auto& sample) {
                             EXPECT_THAT(*sample, Eq(i));
                             EXPECT_THAT(sample.getChunkHeader()->originId(), Eq(uid));
                         })
                         .has_error());
    }
}

#ifdef TEST_WITH_HUGE_PAYLOAD

TEST_F(PublisherSubscriberCommunicationWithBigPayload_test, SendingComplexDataType_BigPayloadStruct)
{
    ::testing::Test::RecordProperty("TEST_ID", "f612a4ef-5f3a-4951-8f2e-bbc28f6b1a66");

    using Type_t = ComplexDataType<BigPayloadStruct>;
    auto publisher = createPublisher<Type_t>();
    auto subscriber = createSubscriber<Type_t>();

    constexpr uint64_t PAGE_SIZE = 4096;

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         for (uint64_t i = PAGE_SIZE - 1; i < SIZE_LARGER_THAN_4GB; i += PAGE_SIZE)
                         {
                             sample->complexType.bigPayload[i] = static_cast<uint8_t>(i / PAGE_SIZE);
                         }
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         for (uint64_t i = PAGE_SIZE - 1; i < SIZE_LARGER_THAN_4GB; i += PAGE_SIZE)
                         {
                             ASSERT_THAT(sample->complexType.bigPayload[i], Eq(static_cast<uint8_t>(i / PAGE_SIZE)));
                         }
                     })
                     .has_error());
}

#endif

} // namespace
