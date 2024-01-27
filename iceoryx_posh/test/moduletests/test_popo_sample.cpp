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

#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "iox/unique_ptr.hpp"

#include "iceoryx_hoofs/testing/error_reporting/testing_support.hpp"
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

    sutProducer.publish();

    EXPECT_FALSE(sutProducer);
}

TEST_F(Sample_test, SendOnMoveDestinationCallsInterfaceMock)
{
    ::testing::Test::RecordProperty("TEST_ID", "74a62eae-d36f-47bf-9df9-695e50fcdd88");
    EXPECT_CALL(mockInterface, mockSend(_)).Times(1);

    auto movedSut = std::move(sutProducer);
    movedSut.publish();

    EXPECT_FALSE(sutProducer);
}

TEST_F(Sample_test, PublishingAlreadyPublishedSampleCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "5b0302c9-814a-4b03-813a-fd5586fc987c");
    EXPECT_CALL(mockInterface, mockSend(_)).Times(1);

    sutProducer.publish();

    IOX_TESTING_EXPECT_OK();

    sutProducer.publish();

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__PUBLISHING_EMPTY_SAMPLE);
}

TEST_F(Sample_test, PublishingMovedSampleCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "4c3a9a19-0581-4e47-aed7-f55892bef7fa");

    auto movedSut = std::move(sutProducer);

    IOX_TESTING_EXPECT_OK();

    sutProducer.publish();

    IOX_TESTING_EXPECT_ERROR(iox::PoshError::POSH__PUBLISHING_EMPTY_SAMPLE);
}

} // namespace
