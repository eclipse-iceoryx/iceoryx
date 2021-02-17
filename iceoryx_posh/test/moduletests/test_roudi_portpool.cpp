// Copyright (c) 2021 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iceoryx_posh/roudi/port_pool.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::Return;

namespace test
{
class PortPool_test : public Test
{
  public:
    iox::roudi::PortPoolData portPoolData;
    iox::roudi::PortPool sut{portPoolData};
};

TEST_F(PortPool_test, AddNodeDataFailsWhenNodeListIsFull)
{
    auto errorHandlerCalled{false};
    iox::Error errorHandlerType;
    auto errorHandlerGuard = iox::ErrorHandler::SetTemporaryErrorHandler(
        [&](const iox::Error error, const std::function<void()>, const iox::ErrorLevel) {
            errorHandlerType = error;
            errorHandlerCalled = true;
        });

    for (uint32_t i = 0U; i <= iox::MAX_NODE_NUMBER; ++i)
    {
        sut.addNodeData("processName", "nameName", i);
    }
    ASSERT_THAT(errorHandlerCalled, Eq(true));
    EXPECT_EQ(errorHandlerType, iox::Error::kPORT_POOL__NODELIST_OVERFLOW);
}

} // namespace test
