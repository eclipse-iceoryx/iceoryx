// Copyright (c) 2024 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include "iox/deadline_timer.hpp"
#include "iox/duration.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "iceoryx_posh/roudi_env/roudi_env_node_builder.hpp"
#include "test.hpp"

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

} // namespace