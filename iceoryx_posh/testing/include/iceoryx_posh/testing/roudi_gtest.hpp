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
#ifndef IOX_POSH_TESTUTILS_ROUDI_GTEST_HPP
#define IOX_POSH_TESTUTILS_ROUDI_GTEST_HPP

#include "iceoryx_posh/roudi_env/roudi_env.hpp"
#include "iox/detail/deprecation_marker.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace iox
{
namespace testing
{
class RouDi_GTest : public iox::roudi_env::RouDiEnv, public ::testing::Test
{
  public:
    RouDi_GTest() = default;
    RouDi_GTest(const iox::IceoryxConfig& config) noexcept;

    /// @deprecated Deprecated in 3.0, removed in 4.0, please port to 'setDiscoveryLoopWaitToFinishTimeout'
    IOX_DEPRECATED_SINCE(3, "Please port to 'setDiscoveryLoopWaitToFinishTimeout'")
    void SetInterOpWaitingTime(const std::chrono::milliseconds& v) noexcept;

    /// @deprecated Deprecated in 3.0, removed in 4.0, please port to 'triggerDiscoveryLoopAndWaitToFinish'
    IOX_DEPRECATED_SINCE(3, "Please port to 'triggerDiscoveryLoopAndWaitToFinish'") void InterOpWait() noexcept;

    /// @deprecated Deprecated in 3.0, removed in 4.0, please port to 'cleanupAppResources'
    IOX_DEPRECATED_SINCE(3, "Please port to 'cleanupAppResources'")
    void CleanupAppResources(const RuntimeName_t& name) noexcept;
};

} // namespace testing
} // namespace iox

/// @deprecated Deprecated in 3.0, removed in 4.0, please use 'RouDi_GTest' with the 'iox::testing' namespace
using RouDi_GTest = iox::testing::RouDi_GTest;

#endif // IOX_POSH_TESTUTILS_ROUDI_GTEST_HPP
