// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/runtime/node.hpp"
#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox::runtime;
using namespace iox::roudi;


namespace iox
{
namespace test
{
/// @brief Test goal: This test suit verifies class node

//bool callbackWasCalled = false;
//PoshRuntime& testFactory(iox::cxx::optional<const iox::ProcessName_t*> name)
//{
//    callbackWasCalled = true;
//    return PoshRuntime::defaultRuntimeFactory(name);
//}

class PoshRuntimeNode_test : public Test
{
  public:
    PoshRuntimeNode_test()
    {
    }

    virtual ~PoshRuntimeNode_test()
    {
    }

    virtual void SetUp()
    {
    };

    virtual void TearDown()
    {
    };

    const ProcessName_t m_runtimeName{"publisher"};
    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};
    PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::initRuntime(m_runtimeName)};
};

TEST_F(PoshRuntimeNode_test, CreateNode)
{
    const NodeName_t& nodeName{"Node"};
    Node m_node("Node");

    EXPECT_THAT(m_node.getNodeName(),Eq(nodeName));
}

TEST_F(PoshRuntimeNode_test, NodeOperator)
{
    const NodeName_t& nodeNewName{"NewNode"};
//    const NodeName_t& nodeNewNameTest{"NewNodeTest"};
    Node m_node("Node");

    m_node = Node("NewNode");

    EXPECT_THAT(m_node.getNodeName(), Eq(nodeNewName));
}

} // namespace test
} // namespace iox 
