// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/unique_ptr.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"

#include "test.hpp"
#include "test_popo_smart_chunk_common.hpp"

namespace
{
using namespace ::testing;
using namespace iox::popo;
using namespace test_smart_chunk_common;
using ::testing::_;

class Sample_test : public SampleTestCase, public Test
{
};

TEST_F(Sample_test, SendCallsInterfaceMockWithSuccessResult)
{
    ::testing::Test::RecordProperty("TEST_ID", "2ddbbcad-704f-4f0a-849c-5db8ac339668");
    EXPECT_CALL(mockInterface, mockSend(_)).Times(1);

    publish(std::move(sutProducer));
}

TEST_F(Sample_test, SendOnMoveDestinationCallsInterfaceMock)
{
    ::testing::Test::RecordProperty("TEST_ID", "74a62eae-d36f-47bf-9df9-695e50fcdd88");
    EXPECT_CALL(mockInterface, mockSend(_)).Times(1);

    auto movedSut = std::move(sutProducer);
    publish(std::move(movedSut));
}
} // namespace
