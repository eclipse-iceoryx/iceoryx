// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/testing/roudi_gtest.hpp"

namespace iox
{
namespace testing
{

RouDi_GTest::RouDi_GTest(const iox::IceoryxConfig& config) noexcept
    : iox::roudi_env::RouDiEnv(config)
{
}

void RouDi_GTest::SetInterOpWaitingTime(const std::chrono::milliseconds& v) noexcept
{
    setDiscoveryLoopWaitToFinishTimeout(units::Duration::fromMilliseconds(v.count()));
}

void RouDi_GTest::InterOpWait() noexcept
{
    triggerDiscoveryLoopAndWaitToFinish();
}

void RouDi_GTest::CleanupAppResources(const RuntimeName_t& name) noexcept
{
    cleanupAppResources(name);
}

} // namespace testing
} // namespace iox
