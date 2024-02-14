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

#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox;
using namespace iox::posh::experimental;
using namespace iox::roudi_env;

TEST(Runtime_test, CreatingRuntimeWithRunningRouDiWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "547fb8bf-ff25-4f86-ab7d-27b4474e2cdc");

    RouDiEnv roudi;

    optional<Runtime> runtime;
    auto runtimeResult = RouDiEnvRuntimeBuilder("foo").create(runtime);

    EXPECT_FALSE(runtimeResult.has_error());
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

    optional<Runtime> runtime1;
    auto runtime1_result = RouDiEnvRuntimeBuilder("foo").create(runtime1);
    EXPECT_FALSE(runtime1_result.has_error());
    runtime1.reset();

    runtime1_result = RouDiEnvRuntimeBuilder("foo").create(runtime1);
    EXPECT_FALSE(runtime1_result.has_error());
}

} // namespace
