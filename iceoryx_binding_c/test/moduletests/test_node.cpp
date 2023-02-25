// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "iceoryx_hoofs/testing/fatal_failure.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"
#include <gtest/gtest.h>

extern "C" {
#include "iceoryx_binding_c/node.h"
#include "iceoryx_binding_c/runtime.h"
}

namespace
{
using namespace iox;
using namespace iox::runtime;
using namespace iox::testing;

class iox_node_test : public RouDi_GTest
{
  public:
    void SetUp()
    {
        iox_runtime_init(m_runtimeName.c_str());
        m_sut = iox_node_create(m_nodeName.c_str());
    }

    void TearDown()
    {
        iox_node_destroy(m_sut);
    }

    std::string m_nodeName = "hypnotoadIsWatchingUs";
    std::string m_runtimeName = "stoepselWillMarrySoon";

    iox_node_t m_sut;
};

TEST_F(iox_node_test, createdNodeHasCorrectNodeName)
{
    ::testing::Test::RecordProperty("TEST_ID", "e104fa1c-675f-4488-b19b-09063da8ed73");
    char name[100];
    ASSERT_EQ(iox_node_get_name(m_sut, name, 100), m_nodeName.size());
    EXPECT_EQ(std::string(name), m_nodeName);
}

TEST_F(iox_node_test, getNodeNameBufferIsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "b35feed7-15e4-4b0b-a87e-8df8b8453c4b");
    auto nameLength = iox_node_get_name(m_sut, nullptr, 100);

    ASSERT_THAT(nameLength, Eq(0U));
}

TEST_F(iox_node_test, getNodeNameBufferIsLessThanNodeNameLength)
{
    ::testing::Test::RecordProperty("TEST_ID", "19dd16d9-b1ee-4275-9df7-4027dd469645");
    constexpr uint64_t NODE_NAME_BUFFER_LENGTH{10};
    char truncatedNodeName[NODE_NAME_BUFFER_LENGTH];
    for (auto& c : truncatedNodeName)
    {
        c = '#';
    }
    auto nameLength = iox_node_get_name(m_sut, truncatedNodeName, NODE_NAME_BUFFER_LENGTH);

    std::string expectedNodeName = "hypnotoad";

    ASSERT_THAT(nameLength, Eq(m_nodeName.size()));
    EXPECT_THAT(truncatedNodeName, StrEq(expectedNodeName));
}

TEST_F(iox_node_test, createdNodeHasCorrectProcessName)
{
    ::testing::Test::RecordProperty("TEST_ID", "ae0d1b9e-38b1-4b50-bf36-770ee32cf518");
    char name[100];
    ASSERT_EQ(iox_node_get_runtime_name(m_sut, name, 100), m_runtimeName.size());
    EXPECT_EQ(std::string(name), m_runtimeName);
}

TEST_F(iox_node_test, getNodeRuntimeNameBufferIsNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "a684e40b-4ac9-4f79-945f-e94aee778e82");
    auto nameLength = iox_node_get_runtime_name(m_sut, nullptr, 100);

    ASSERT_THAT(nameLength, Eq(0U));
}

TEST_F(iox_node_test, getNodeRuntimeNameBufferIsLessThanNodeProcessNameLength)
{
    ::testing::Test::RecordProperty("TEST_ID", "e9ef5e9c-353f-4c7d-8a30-13a277e1adcd");
    constexpr uint64_t PROCESS_NAME_BUFFER_LENGTH{9};
    char truncatedProcessName[PROCESS_NAME_BUFFER_LENGTH];
    for (auto& c : truncatedProcessName)
    {
        c = '#';
    }
    auto nameLength = iox_node_get_runtime_name(m_sut, truncatedProcessName, PROCESS_NAME_BUFFER_LENGTH);

    std::string expectedProcessName = "stoepsel";

    ASSERT_THAT(nameLength, Eq(m_runtimeName.size()));
    EXPECT_THAT(truncatedProcessName, StrEq(expectedProcessName));
}

TEST_F(iox_node_test, destroyNodeWithNullptrFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "c5665aef-0710-4300-a99f-f968739e5222");
    IOX_EXPECT_FATAL_FAILURE<iox::HoofsError>([&] { iox_node_destroy(nullptr); },
                                              iox::HoofsError::EXPECTS_ENSURES_FAILED);
}

} // namespace
