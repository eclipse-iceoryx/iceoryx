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

#include "iox/posh/experimental/runtime.hpp"

#include "iox/deadline_timer.hpp"
#include "iox/duration.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox;
using namespace iox::posh::experimental;
using namespace iox::roudi_env;
using namespace iox::units::duration_literals;

TEST(Runtime_test, CreatingRuntimeWithRunningRouDiWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "547fb8bf-ff25-4f86-ab7d-27b4474e2cdc");

    RouDiEnv roudi;

    optional<Runtime> runtime;
    auto runtime_result = RouDiEnvRuntimeBuilder("foo").create(runtime);

    EXPECT_FALSE(runtime_result.has_error());
}

TEST(Runtime_test, CreatingMultipleRuntimesWithRunningRouDiWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "8fe6c62f-7aa0-4822-b5e3-974b4e91c7b7");

    RouDiEnv roudi;

    optional<Runtime> runtime1;
    auto runtime1_result = RouDiEnvRuntimeBuilder("foo").create(runtime1);

    optional<Runtime> runtime2;
    auto runtime2_result = RouDiEnvRuntimeBuilder("bar").create(runtime2);

    EXPECT_FALSE(runtime1_result.has_error());
    EXPECT_FALSE(runtime2_result.has_error());
}

TEST(Runtime_test, ReRegisteringRuntimeWithRunningRouDiWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ce9d5f0-6989-4302-92b7-458fe1412111");

    RouDiEnv roudi;

    optional<Runtime> runtime;
    auto runtime_result = RouDiEnvRuntimeBuilder("foo").create(runtime);
    EXPECT_FALSE(runtime_result.has_error());
    runtime.reset();

    runtime_result = RouDiEnvRuntimeBuilder("foo").create(runtime);
    EXPECT_FALSE(runtime_result.has_error());
}

TEST(Runtime_test, RegisteringRuntimeWithoutRunningRouDiWithZeroWaitTimeResultsInImmediateTimeout)
{
    ::testing::Test::RecordProperty("TEST_ID", "f2041773-84d9-4c9b-9309-996af83d6ff0");

    deadline_timer timer{20_ms};

    optional<Runtime> runtime;
    auto runtime_result = RouDiEnvRuntimeBuilder("foo").create(runtime);

    EXPECT_FALSE(timer.hasExpired());

    ASSERT_TRUE(runtime_result.has_error());
    EXPECT_THAT(runtime_result.error(), Eq(RuntimeBuilder::Error::TIMEOUT));
}

TEST(Runtime_test, RegisteringRuntimeWithoutRunningRouDiWithSomeWaitTimeResultsInTimeout)
{
    ::testing::Test::RecordProperty("TEST_ID", "ac069a39-6cdc-4f2e-8b88-984a7d1a5487");

    units::Duration wait_for_roudi_test_timeout{100_ms};
    units::Duration wait_for_roudi_timeout{2 * wait_for_roudi_test_timeout};
    deadline_timer timer{wait_for_roudi_test_timeout};

    optional<Runtime> runtime;
    auto runtime_result =
        RouDiEnvRuntimeBuilder("foo").roudi_registration_timeout(wait_for_roudi_timeout).create(runtime);

    EXPECT_TRUE(timer.hasExpired());

    ASSERT_TRUE(runtime_result.has_error());
    EXPECT_THAT(runtime_result.error(), Eq(RuntimeBuilder::Error::TIMEOUT));
}

TEST(Runtime_test, RegisteringRuntimeWithDelayedRouDiStartWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "63ef9a1a-deee-40b5-bc17-37ee67ad8d76");

    optional<Runtime> runtime;
    auto runtime_result = RouDiEnvRuntimeBuilder("foo").create(runtime);

    ASSERT_TRUE(runtime_result.has_error());
    EXPECT_THAT(runtime_result.error(), Eq(RuntimeBuilder::Error::TIMEOUT));

    RouDiEnv roudi;

    runtime_result = RouDiEnvRuntimeBuilder("foo").create(runtime);

    EXPECT_FALSE(runtime_result.has_error());
}

TEST(Runtime_test, CreatingPublisherWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c98d1cb6-8990-4f91-a24b-d845d2dc37e1");

    RouDiEnv roudi;

    optional<Runtime> runtime;
    RouDiEnvRuntimeBuilder("hypnotoad").create(runtime).expect("Creating a runtime should not fail!");

    auto maybe_publisher = runtime->publisher({"all", "glory", "hypnotoad"}).create<uint8_t>();
    ASSERT_FALSE(maybe_publisher.has_error());

    IOX_LOG(INFO, "Move it!");

    auto publisher = std::move(maybe_publisher.value());

    IOX_TESTING_ASSERT_NO_PANIC();
}

TEST(Runtime_test, CreatingSubscriberWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e14f3c82-d758-43cc-bd89-dfdf0ed71480");

    RouDiEnv roudi;

    optional<Runtime> runtime;
    RouDiEnvRuntimeBuilder("hypnotoad").create(runtime).expect("Creating a runtime should not fail!");

    auto maybe_subscriber = runtime->subscriber({"all", "glory", "hypnotoad"}).create<uint8_t>();
    ASSERT_FALSE(maybe_subscriber.has_error());

    IOX_LOG(INFO, "Move it!");

    auto subscriber = std::move(maybe_subscriber.value());

    IOX_TESTING_ASSERT_NO_PANIC();
}

} // namespace
