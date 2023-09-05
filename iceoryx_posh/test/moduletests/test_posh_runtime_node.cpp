// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/runtime/node.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/testing/roudi_environment/minimal_roudi_config.hpp"
#include "iceoryx_posh/testing/roudi_environment/roudi_environment.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::runtime;
using namespace iox::roudi;
using namespace iox::testing;
using namespace iox;

/// @brief Test goal: This test suit verifies class node

class PoshRuntimeNode_test : public Test
{
  public:
    PoshRuntimeNode_test()
    {
    }

    virtual ~PoshRuntimeNode_test()
    {
    }

    virtual void SetUp(){};

    virtual void TearDown(){};

    const RuntimeName_t m_runtimeName{"App"};
    RouDiEnvironment m_roudiEnv{MinimalRouDiConfigBuilder().create()};
    PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::initRuntime(m_runtimeName)};
};

TEST_F(PoshRuntimeNode_test, ConstructorNodeIsSuccess)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bba69cc-43ea-47d3-9207-08afdd7eed9b");
    const NodeName_t nodeName{"Node"};

    Node node("Node");

    EXPECT_THAT(node.getNodeName(), Eq(nodeName));
}

TEST_F(PoshRuntimeNode_test, ConstructorNodeEmptyNodeNameIsSuccess)
{
    ::testing::Test::RecordProperty("TEST_ID", "c1620584-9676-415d-af7a-a3f7263bafee");
    const NodeName_t nodeName{""};

    Node node("");

    EXPECT_THAT(node.getNodeName(), Eq(nodeName));
}

TEST_F(PoshRuntimeNode_test, ConstructorNodeWithMaximalSizeNodeNameIsSuccess)
{
    ::testing::Test::RecordProperty("TEST_ID", "286fa814-6681-411f-9ef9-924da4f4af28");
    const NodeName_t nodeName{
        "aaaaabbbbbcccccdddddaaaaabbbbbcccccdddddaaaaabbbbbcccccdddddaaaaabbbbbcccccdddddaaaaabbbbbcccccddddd"};

    Node node("aaaaabbbbbcccccdddddaaaaabbbbbcccccdddddaaaaabbbbbcccccdddddaaaaabbbbbcccccdddddaaaaabbbbbcccccddddd");

    EXPECT_THAT(node.getNodeName(), Eq(nodeName));
}

TEST_F(PoshRuntimeNode_test, VerifyMoveAssignmentOperatorAssignsCorrectName)
{
    ::testing::Test::RecordProperty("TEST_ID", "22b51fc1-90d3-4d5b-8004-a2da3d8eb5f7");
    const NodeName_t nodeName{"@!~*"};
    Node testNode(nodeName);
    Node node("Node");

    node = std::move(testNode);

    EXPECT_THAT(node.getNodeName(), Eq(nodeName));
}

TEST_F(PoshRuntimeNode_test, SelfMoveAssignmentIsExcluded)
{
    ::testing::Test::RecordProperty("TEST_ID", "10be17a2-6253-4f16-befb-08d72379d892");
    const NodeName_t nodeName{"Node"};
    Node node1(nodeName);
    Node& node2 = node1;

    node1 = std::move(node2);

    EXPECT_THAT(node1.getNodeName(), Eq(nodeName));
}

TEST_F(PoshRuntimeNode_test, VerifyMoveConstructorAssignsCorrectNodeName)
{
    ::testing::Test::RecordProperty("TEST_ID", "9322a724-7da1-4728-bff3-fa0adc2a0855");
    const NodeName_t nodeNewName{"Node"};

    Node node(nodeNewName);

    Node nodeTest(std::move(node));

    EXPECT_THAT(nodeTest.getNodeName(), Eq(nodeNewName));
}

} // namespace
