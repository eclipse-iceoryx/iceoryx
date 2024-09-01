// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iox/posh/experimental/node.hpp"

#include "iceoryx_platform/stdlib.hpp"
#include "iox/deadline_timer.hpp"
#include "iox/detail/system_configuration.hpp"
#include "iox/duration.hpp"
#include "iox/vector.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "iceoryx_posh/roudi_env/roudi_env_node_builder.hpp"
#include "test.hpp"

#include <optional>

namespace
{
using namespace ::testing;

using namespace iox;
using namespace iox::posh::experimental;
using namespace iox::roudi_env;
using namespace iox::units::duration_literals;

struct Payload
{
};
struct Header
{
};

TEST(Node_test, CreatingNodeWithRunningRouDiWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "547fb8bf-ff25-4f86-ab7d-27b4474e2cdc");

    RouDiEnv roudi;

    auto node_result = RouDiEnvNodeBuilder("foo").create();

    ASSERT_FALSE(node_result.has_error());

    auto node = std::move(node_result.value());

    IOX_TESTING_ASSERT_NO_PANIC();
}

TEST(Node_test, CreatingMultipleNodesWithRunningRouDiWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fe6c62f-7aa0-4822-b5e3-974b4e91c7b7");

    RouDiEnv roudi;

    auto node1_result = RouDiEnvNodeBuilder("foo").create();
    auto node2_result = RouDiEnvNodeBuilder("bar").create();

    ASSERT_FALSE(node1_result.has_error());
    ASSERT_FALSE(node2_result.has_error());

    auto node1 = std::move(node1_result.value());
    auto node2 = std::move(node2_result.value());

    IOX_TESTING_ASSERT_NO_PANIC();
}

TEST(Node_test, NodeDoesNotUseTheStaticRuntime)
{
    ::testing::Test::RecordProperty("TEST_ID", "9408ea42-38ab-4547-b7b3-ec2dda2501ba");

    RouDiEnv roudi;

    auto node1 = RouDiEnvNodeBuilder("foo").create().expect("Creating a node should not fail!");
    auto node2 = RouDiEnvNodeBuilder("bar").create().expect("Creating a node should not fail!");

    EXPECT_THAT(roudi.numberOfActiveRuntimeTestInterfaces(), Eq(0));
}

TEST(Node_test, CreatingNodeWithInvalidNameLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "7a460f65-2970-489f-98e3-2c402fb05766");

    RouDiEnv roudi;

    RouDiEnvNodeBuilder("")
        .create()
        .and_then([](const auto&) { GTEST_FAIL() << "Creating a 'Node' with empty name should fail"; })
        .or_else([](const auto error) { EXPECT_THAT(error, Eq(NodeBuilderError::IPC_CHANNEL_CREATION_FAILED)); });

    RouDiEnvNodeBuilder("/foo")
        .create()
        .and_then([](const auto&) { GTEST_FAIL() << "Creating a 'Node' with '/' in name should fail"; })
        .or_else([](const auto error) { EXPECT_THAT(error, Eq(NodeBuilderError::IPC_CHANNEL_CREATION_FAILED)); });
}

TEST(Node_test, CreatingSameNodeTwiceLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "5627075d-4537-4bd1-95de-136549fc02a7");

    RouDiEnv roudi;

    auto node1_result = RouDiEnvNodeBuilder("foo").create();
    auto node2_result = RouDiEnvNodeBuilder("foo").create();

    ASSERT_FALSE(node1_result.has_error());
    ASSERT_TRUE(node2_result.has_error());

    EXPECT_THAT(node2_result.error(), Eq(NodeBuilderError::IPC_CHANNEL_CREATION_FAILED));
}

TEST(Node_test, CreatingNodeWithDomainIdFromEnvFailsIfDomainIdIsNotSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1268403-2b76-4713-a4f6-5f62a9ce9e57");

    IOX_POSIX_CALL(iox_unsetenv)
    ("IOX_DOMAIN_ID").failureReturnValue(-1).evaluate().expect("Unsetting environment variable works!");
    auto node_result = RouDiEnvNodeBuilder("foo").domain_id_from_env().create();

    ASSERT_TRUE(node_result.has_error());
    EXPECT_THAT(node_result.error(), Eq(NodeBuilderError::INVALID_OR_NO_DOMAIN_ID));
}

TEST(Node_test, CreatingNodeWithDomainIdFromEnvFailsIfDomainIdIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "07bc4bf6-cb06-40cb-b3d4-761e95e82e4b");

    constexpr int32_t OVERWRITE_ENV_VARIABLE{1};
    IOX_POSIX_CALL(iox_setenv)
    ("IOX_DOMAIN_ID", "1234567", OVERWRITE_ENV_VARIABLE)
        .failureReturnValue(-1)
        .evaluate()
        .expect("Setting environment variable works!");
    auto node_result = RouDiEnvNodeBuilder("foo").domain_id_from_env().create();

    ASSERT_TRUE(node_result.has_error());
    EXPECT_THAT(node_result.error(), Eq(NodeBuilderError::INVALID_OR_NO_DOMAIN_ID));
}

TEST(Node_test, CreatingNodeWithDomainIdFromEnvWorksIfDomainIdIsSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "dcf02c88-8c7a-4327-8ba2-0f71dc7b0ff1");

    RouDiEnv roudi{DomainId{42}};

    constexpr int32_t OVERWRITE_ENV_VARIABLE{1};
    IOX_POSIX_CALL(iox_setenv)
    ("IOX_DOMAIN_ID", "42", OVERWRITE_ENV_VARIABLE)
        .failureReturnValue(-1)
        .evaluate()
        .expect("Setting environment variable works!");
    auto node_result = RouDiEnvNodeBuilder("foo").domain_id_from_env().create();

    EXPECT_FALSE(node_result.has_error());
}

TEST(Node_test, CreatingNodeWithDomainIdFromEnvOrAlternativeValueWorksIfDomainIdIsSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "ba16d5cc-46b8-4450-8c77-16081a52f38c");

    RouDiEnv roudi{DomainId{42}};

    constexpr int32_t OVERWRITE_ENV_VARIABLE{1};
    IOX_POSIX_CALL(iox_setenv)
    ("IOX_DOMAIN_ID", "42", OVERWRITE_ENV_VARIABLE)
        .failureReturnValue(-1)
        .evaluate()
        .expect("Setting environment variable works!");
    auto node_result = RouDiEnvNodeBuilder("foo").domain_id_from_env_or(DomainId{13}).create();

    EXPECT_FALSE(node_result.has_error());
}

TEST(Node_test, CreatingNodeWithDomainIdFromEnvOrAlternativeValueWorksIfDomainIdIsNotSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "b071843a-a821-43b4-ac1a-e76ccafd35e0");

    RouDiEnv roudi{DomainId{13}};

    IOX_POSIX_CALL(iox_unsetenv)
    ("IOX_DOMAIN_ID").failureReturnValue(-1).evaluate().expect("Unsetting environment variable works!");
    auto node_result = RouDiEnvNodeBuilder("foo").domain_id_from_env_or(DomainId{13}).create();

    EXPECT_FALSE(node_result.has_error());
}

TEST(Node_test, CreatingNodeWithDomainIdFromEnvOrDefaultWorksIfDomainIdIsSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "35f422ec-3723-4c8f-93ae-ce1c8dfaca76");

    RouDiEnv roudi{DomainId{42}};

    constexpr int32_t OVERWRITE_ENV_VARIABLE{1};
    IOX_POSIX_CALL(iox_setenv)
    ("IOX_DOMAIN_ID", "42", OVERWRITE_ENV_VARIABLE)
        .failureReturnValue(-1)
        .evaluate()
        .expect("Setting environment variable works!");
    auto node_result = RouDiEnvNodeBuilder("foo").domain_id_from_env_or_default().create();

    EXPECT_FALSE(node_result.has_error());
}

TEST(Node_test, CreatingNodeWithDomainIdFromEnvOrDefaultWorksIfDomainIdIsNotSet)
{
    ::testing::Test::RecordProperty("TEST_ID", "363dfb49-75fa-4486-b8b1-0f31c16bf37c");

    RouDiEnv roudi{DEFAULT_DOMAIN_ID};

    IOX_POSIX_CALL(iox_unsetenv)
    ("IOX_DOMAIN_ID").failureReturnValue(-1).evaluate().expect("Unsetting environment variable works!");
    auto node_result = RouDiEnvNodeBuilder("foo").domain_id_from_env_or_default().create();

    EXPECT_FALSE(node_result.has_error());
}

TEST(Node_test, ExhaustingNodesLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "540aa751-cf7b-43fb-800b-a05d3eacf68e");

    bool run_ulimit_test{false};

    if (const auto* run_ulimit_test_string = getenv("IOX_RUN_ULIMIT_TESTS"))
    {
        if (strncmp(run_ulimit_test_string, "on", 2) == 0)
        {
            run_ulimit_test = true;
        }
        else if (strncmp(run_ulimit_test_string, "off", 2) == 0)
        {
            run_ulimit_test = false;
        }
        else
        {
            std::cout << "Invalid value for 'IOX_RUN_ULIMIT_TESTS' environment variable!'" << std::endl;
            std::cout << "Found:" << run_ulimit_test_string << std::endl;
            std::cout << "Allowed is either 'on' of 'off'!";
        }
    }

    if (!run_ulimit_test)
    {
        GTEST_SKIP() << "Set the 'IOX_RUN_ULIMIT_TESTS' env variable to 'on' to run this test. It might fail if "
                        "number of file descriptors is not increased with 'ulimit -n 2000'!";
    }

    if (iox::detail::isCompiledOn32BitSystem())
    {
        GTEST_SKIP() << "@todo iox-#2301 This test fails on 32 bit builds on the CI after ~240 created Nodes. "
                        "Potentially some issues with the amount of file descriptors.";
    }

    RouDiEnv roudi;

    iox::vector<Node, iox::MAX_NODE_NUMBER> nodes;

    for (uint64_t i = 0; i < iox::MAX_NODE_NUMBER; ++i)
    {
        nodes.emplace_back(RouDiEnvNodeBuilder(NodeName_t(iox::TruncateToCapacity, iox::convert::toString(i).c_str()))
                               .create()
                               .expect("Creating a node should not fail!"));
    }

    auto node_result = RouDiEnvNodeBuilder("hypnotoad").create();
    ASSERT_TRUE(node_result.has_error());
    EXPECT_THAT(node_result.error(), Eq(NodeBuilderError::REGISTRATION_FAILED));
}

TEST(Node_test, ReRegisteringNodeWithRunningRouDiWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ce9d5f0-6989-4302-92b7-458fe1412111");

    RouDiEnv roudi;

    optional<Node> node;

    auto node_result = RouDiEnvNodeBuilder("foo").create();
    ASSERT_FALSE(node_result.has_error());

    node.emplace(std::move(node_result.value()));
    node.reset();

    node_result = RouDiEnvNodeBuilder("foo").create();
    ASSERT_FALSE(node_result.has_error());

    node.emplace(std::move(node_result.value()));

    IOX_TESTING_ASSERT_NO_PANIC();
}

TEST(Node_test, RegisteringNodeWithoutRunningRouDiWithZeroWaitTimeResultsInImmediateTimeout)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2041773-84d9-4c9b-9309-996af83d6ff0");

    deadline_timer timer{20_ms};

    auto node_result = RouDiEnvNodeBuilder("foo").create();

    EXPECT_FALSE(timer.hasExpired());

    ASSERT_TRUE(node_result.has_error());
    EXPECT_THAT(node_result.error(), Eq(NodeBuilderError::TIMEOUT));
}

TEST(Node_test, RegisteringNodeWithoutRunningRouDiWithSomeWaitTimeResultsInTimeout)
{
    ::testing::Test::RecordProperty("TEST_ID", "ac069a39-6cdc-4f2e-8b88-984a7d1a5487");

    units::Duration wait_for_roudi_test_timeout{100_ms};
    units::Duration wait_for_roudi_timeout{2 * wait_for_roudi_test_timeout};
    deadline_timer timer{wait_for_roudi_test_timeout};

    auto node_result = RouDiEnvNodeBuilder("foo").roudi_registration_timeout(wait_for_roudi_timeout).create();

    EXPECT_TRUE(timer.hasExpired());

    ASSERT_TRUE(node_result.has_error());
    EXPECT_THAT(node_result.error(), Eq(NodeBuilderError::TIMEOUT));
}

TEST(Node_test, RegisteringNodeWithDelayedRouDiStartWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "63ef9a1a-deee-40b5-bc17-37ee67ad8d76");

    iox::optional<RouDiEnv> roudi;

    auto node_result = RouDiEnvNodeBuilder("foo").create();

    ASSERT_TRUE(node_result.has_error());
    EXPECT_THAT(node_result.error(), Eq(NodeBuilderError::TIMEOUT));

    roudi.emplace();

    node_result = RouDiEnvNodeBuilder("foo").create();

    EXPECT_FALSE(node_result.has_error());
}

TEST(Node_test, RegisteringNodeWithRunningRouDiWithNonMatchingDomainIdResultsInTimeout)
{
    ::testing::Test::RecordProperty("TEST_ID", "c61390ac-3245-4cf7-ba13-608a07ea5ffa");

    RouDiEnv roudi{DomainId{42}};

    auto node_result = RouDiEnvNodeBuilder("foo").domain_id(DomainId{13}).create();

    ASSERT_TRUE(node_result.has_error());
    EXPECT_THAT(node_result.error(), Eq(NodeBuilderError::TIMEOUT));
}

TEST(Node_test, CreatingTypedPublisherWithoutUserHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c98d1cb6-8990-4f91-a24b-d845d2dc37e1");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    auto publisher_result = node.publisher({"all", "glory", "hypnotoad"}).create<Payload>();
    ASSERT_FALSE(publisher_result.has_error());

    auto publisher = std::move(publisher_result.value());

    EXPECT_TRUE((std::is_same_v<decltype(publisher), iox::unique_ptr<iox::posh::experimental::Publisher<Payload>>>));
}

TEST(Node_test, CreatingTypedPublisherWithUserHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6b3d2144-1048-4fc4-91c0-4e251c608bf0");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    auto publisher_result = node.publisher({"all", "glory", "hypnotoad"}).create<Payload, Header>();
    ASSERT_FALSE(publisher_result.has_error());

    auto publisher = std::move(publisher_result.value());

    EXPECT_TRUE(
        (std::is_same_v<decltype(publisher), iox::unique_ptr<iox::posh::experimental::Publisher<Payload, Header>>>));
}

TEST(Node_test, CreatingUntypedPublisherWithUserHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b00fabef-71ee-42bc-bf7c-7c772de00008");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    auto publisher_result = node.publisher({"all", "glory", "hypnotoad"}).create();
    ASSERT_FALSE(publisher_result.has_error());

    auto publisher = std::move(publisher_result.value());

    EXPECT_TRUE((std::is_same_v<decltype(publisher), iox::unique_ptr<iox::posh::experimental::UntypedPublisher>>));
}

TEST(Node_test, CreatingTypedSubscriberWithoutUserHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e14f3c82-d758-43cc-bd89-dfdf0ed71480");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    auto subscriber_result = node.subscriber({"all", "glory", "hypnotoad"}).create<Payload>();
    ASSERT_FALSE(subscriber_result.has_error());

    auto subscriber = std::move(subscriber_result.value());

    EXPECT_TRUE((std::is_same_v<decltype(subscriber), iox::unique_ptr<iox::posh::experimental::Subscriber<Payload>>>));
}

TEST(Node_test, CreatingTypedSubscriberWithUserHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f3181e5-989d-4c61-8507-3f224027407c");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    auto subscriber_result = node.subscriber({"all", "glory", "hypnotoad"}).create<Payload, Header>();
    ASSERT_FALSE(subscriber_result.has_error());


    auto subscriber = std::move(subscriber_result.value());

    EXPECT_TRUE(
        (std::is_same_v<decltype(subscriber), iox::unique_ptr<iox::posh::experimental::Subscriber<Payload, Header>>>));
}

TEST(Node_test, CreatingUntypedSubscriberWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1bb7dab1-fe5c-4a37-98b2-cad229fdbca0");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    auto subscriber_result = node.subscriber({"all", "glory", "hypnotoad"}).create();
    ASSERT_FALSE(subscriber_result.has_error());

    auto subscriber = std::move(subscriber_result.value());

    EXPECT_TRUE((std::is_same_v<decltype(subscriber), iox::unique_ptr<iox::posh::experimental::UntypedSubscriber>>));
}

TEST(Node_test, CreatingWaitSetWithDefaultCapacityWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ccbef3ca-87b5-4d76-955e-171c5f1b5abd");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    auto ws_result = node.wait_set().create();
    ASSERT_FALSE(ws_result.has_error());

    auto ws = std::move(ws_result.value());

    EXPECT_TRUE((std::is_same_v<decltype(ws), iox::unique_ptr<iox::posh::experimental::WaitSet<>>>));
}

TEST(Node_test, CreatingWaitSetWithCustomCapacityWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "248b8130-9d26-44a9-874f-a31a7b415ed6");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    constexpr uint64_t CAPACITY{42};
    auto ws_result = node.wait_set().create<CAPACITY>();
    ASSERT_FALSE(ws_result.has_error());

    auto ws = std::move(ws_result.value());

    EXPECT_TRUE((std::is_same_v<decltype(ws), iox::unique_ptr<iox::posh::experimental::WaitSet<CAPACITY>>>));
}

TEST(Node_test, ExhaustingPublisherSubscriberAndWaitSetLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "794e5db8-8d08-428b-af21-e3934a29ea8f");

    const ServiceDescription service_description{"all", "glory", "hypnotoad"};

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    iox::vector<iox::unique_ptr<UntypedPublisher>, iox::MAX_PUBLISHERS> pub;
    for (uint64_t i = 0; i < iox::MAX_PUBLISHERS - iox::NUMBER_OF_INTERNAL_PUBLISHERS; ++i)
    {
        pub.emplace_back(node.publisher(service_description).create().expect("Getting publisher"));
    }

    iox::vector<iox::unique_ptr<UntypedSubscriber>, iox::MAX_SUBSCRIBERS> sub;
    for (uint64_t i = 0; i < iox::MAX_SUBSCRIBERS; ++i)
    {
        sub.emplace_back(node.subscriber(service_description).create().expect("Getting subscriber"));
    }

    iox::vector<iox::unique_ptr<WaitSet<>>, iox::MAX_NUMBER_OF_CONDITION_VARIABLES> ws;
    for (uint64_t i = 0; i < iox::MAX_NUMBER_OF_CONDITION_VARIABLES; ++i)
    {
        ws.emplace_back(node.wait_set().create().expect("Getting waitset"));
    }

    auto publisher_result = node.publisher(service_description).create();
    ASSERT_TRUE(publisher_result.has_error());
    EXPECT_THAT(publisher_result.error(), Eq(PublisherBuilderError::OUT_OF_RESOURCES));

    auto subscriber_result = node.subscriber(service_description).create();
    ASSERT_TRUE(subscriber_result.has_error());
    EXPECT_THAT(subscriber_result.error(), Eq(SubscriberBuilderError::OUT_OF_RESOURCES));

    auto ws_result = node.wait_set().create();
    ASSERT_TRUE(ws_result.has_error());
    EXPECT_THAT(ws_result.error(), Eq(WaitSetBuilderError::OUT_OF_RESOURCES));
}

TEST(Node_test, PublisherAndSubscriberAreConnected)
{
    ::testing::Test::RecordProperty("TEST_ID", "bafbaebf-e111-4ff0-82e1-53cea1b770f4");

    RouDiEnv roudi;

    auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

    auto publisher = node.publisher({"all", "glory", "hypnotoad"}).create<uint64_t>().expect("Getting publisher");
    auto subscriber = node.subscriber({"all", "glory", "hypnotoad"}).create<uint64_t>().expect("Getting subscriber");

    constexpr uint64_t DATA{42};
    publisher->publishCopyOf(DATA).or_else([](const auto) { GTEST_FAIL() << "Expected to send data"; });
    subscriber->take().and_then([&](const auto& sample) { EXPECT_THAT(*sample, Eq(DATA)); }).or_else([](const auto) {
        GTEST_FAIL() << "Expected to receive data";
    });
}

TEST(Node_test, NodeAndEndpointsAreContinuouslyRecreated)
{
    ::testing::Test::RecordProperty("TEST_ID", "24d93901-0bd5-4458-bb53-7d40e4fb2964");

    RouDiEnv roudi;

    for (uint64_t i = 0; i < 10; ++i)
    {
        auto node = RouDiEnvNodeBuilder("hypnotoad").create().expect("Creating a node should not fail!");

        auto publisher = node.publisher({"all", "glory", "hypnotoad"}).create<uint64_t>().expect("Getting publisher");
        auto subscriber =
            node.subscriber({"all", "glory", "hypnotoad"}).create<uint64_t>().expect("Getting subscriber");

        constexpr uint64_t DATA{42};
        publisher->publishCopyOf(DATA + i).or_else([](const auto) { GTEST_FAIL() << "Expected to send data"; });
        subscriber->take()
            .and_then([&](const auto& sample) { EXPECT_THAT(*sample, Eq(DATA + i)); })
            .or_else([](const auto) { GTEST_FAIL() << "Expected to receive data"; });
    }
}

TEST(Node_test, MultipleNodeAndEndpointsAreRegisteredWithSeparateRouDiRunningInParallel)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e527815-28d1-4a99-a9a3-cc4084018cf3");

    NodeName_t node_name{"hypnotoad"};
    ServiceDescription service_description{"all", "glory", "hypnotoad"};

    constexpr uint16_t domain_id_a{13};
    constexpr uint16_t domain_id_b{42};

    RouDiEnv roudi_a{DomainId{domain_id_a}};
    RouDiEnv roudi_b{DomainId{domain_id_b}};

    auto node_a = RouDiEnvNodeBuilder(node_name)
                      .domain_id(DomainId{domain_id_a})
                      .create()
                      .expect("Creating a node should not fail!");
    auto node_b = RouDiEnvNodeBuilder(node_name)
                      .domain_id(DomainId{domain_id_b})
                      .create()
                      .expect("Creating a node should not fail!");

    auto publisher_a = node_a.publisher(service_description).create<uint16_t>().expect("Getting publisher");
    auto publisher_b = node_b.publisher(service_description).create<uint16_t>().expect("Getting publisher");

    auto subscriber_a = node_a.subscriber(service_description).create<uint16_t>().expect("Getting subscriber");
    auto subscriber_b = node_b.subscriber(service_description).create<uint16_t>().expect("Getting subscriber");

    publisher_a->publishCopyOf(domain_id_a).or_else([](const auto) { GTEST_FAIL() << "Expected to send data"; });
    publisher_b->publishCopyOf(domain_id_b).or_else([](const auto) { GTEST_FAIL() << "Expected to send data"; });

    subscriber_a->take()
        .and_then([&](const auto& sample) { EXPECT_THAT(*sample, Eq(domain_id_a)); })
        .or_else([](const auto) { GTEST_FAIL() << "Expected to receive data"; });
    subscriber_a->take()
        .and_then([&](const auto& sample) { GTEST_FAIL() << "Expected to receive no data but got: " << *sample; })
        .or_else([](const auto) { GTEST_SUCCEED() << "Successfully received no data"; });

    subscriber_b->take()
        .and_then([&](const auto& sample) { EXPECT_THAT(*sample, Eq(domain_id_b)); })
        .or_else([](const auto) { GTEST_FAIL() << "Expected to receive data"; });
    subscriber_b->take()
        .and_then([&](const auto& sample) { GTEST_FAIL() << "Expected to receive no data but got: " << *sample; })
        .or_else([](const auto) { GTEST_SUCCEED() << "Successfully received no data"; });
}

} // namespace
