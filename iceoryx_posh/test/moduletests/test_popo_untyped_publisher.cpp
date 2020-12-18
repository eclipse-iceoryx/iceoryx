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

#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "mocks/chunk_mock.hpp"
#include "mocks/publisher_mock.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

using TestUntypedPublisher = iox::popo::UntypedPublisherImpl<MockBasePublisher<void>>;

class UntypedPublisherTest : public Test
{
  public:
    UntypedPublisherTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

  protected:
    ChunkMock<uint64_t> chunkMock;
    TestUntypedPublisher sut{{"", "", ""}};
};

TEST_F(UntypedPublisherTest, PublishesVoidPointerViaUnderlyingPort)
{
    // ===== Setup ===== //
    EXPECT_CALL(sut.m_port, sendChunk).Times(1); // m_port is mocked.
    // ===== Test ===== //
    sut.publish(chunkMock.chunkHeader()->payload());
    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
