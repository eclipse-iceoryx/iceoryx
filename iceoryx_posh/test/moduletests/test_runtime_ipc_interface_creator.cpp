// Copyright (c) 2021 by Apex.AI. All rights reserved.
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

#if !defined(_WIN32)
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/internal/runtime/ipc_interface_creator.hpp"

#include "test.hpp"

#include <chrono>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::posix;
using namespace iox::runtime;
using namespace iox::testing;

constexpr char goodName[] = "channel_test";
constexpr char anotherGoodName[] = "horst";

/// @req
/// @brief This test suite verifies the additional functionality of IpcInterfaceCreator
/// @pre Make sure no left-over IpcChannels with the names used in this test exist
/// @post None
/// @note Specific functionality of the base class are intentionally not considered in this test
class IpcInterfaceCreator_test : public Test
{
  public:
    void SetUp()
    {
        EXPECT_FALSE(platform::IoxIpcChannelType::unlinkIfExists(goodName).has_error());
        EXPECT_FALSE(platform::IoxIpcChannelType::unlinkIfExists(anotherGoodName).has_error());
    }

    void TearDown()
    {
    }

    ~IpcInterfaceCreator_test()
    {
    }
};

TEST_F(IpcInterfaceCreator_test, CreateWithDifferentNameWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4fc22f1f-1333-41a1-8709-4b1ca791a2e1");
    IpcInterfaceCreator m_sut{goodName};
    IpcInterfaceCreator m_sut2{anotherGoodName};
    EXPECT_TRUE(m_sut.isInitialized());
    EXPECT_TRUE(m_sut2.isInitialized());
}

TEST_F(IpcInterfaceCreator_test, CreateWithSameNameLeadsToError)
{
    ::testing::Test::RecordProperty("TEST_ID", "2e8c15c8-1b7b-465b-aae5-6db24fc3c34a");
    IpcInterfaceCreator m_sut{goodName};

    IOX_EXPECT_FATAL_FAILURE<iox::PoshError>([&] { IpcInterfaceCreator m_sut2{goodName}; },
                                             iox::PoshError::IPC_INTERFACE__APP_WITH_SAME_NAME_STILL_RUNNING);
}

} // namespace

#endif
