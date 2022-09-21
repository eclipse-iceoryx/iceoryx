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

#include "test.hpp"
#include "test_popo_smart_chunk_common.hpp"

namespace
{
using namespace ::testing;
using namespace test_smart_chunk_common;
using ::testing::_;

template <typename T>
class SmartChunkTest : public Test
{
  public:
    using SutProducerType = typename T::ProducerType;
    using SutConsumerType = typename T::ConsumerType;

    T variation;
};

using Implementations = Types<SampleTestCase, RequestTestCase, ResponseTestCase>;

TYPED_TEST_SUITE(SmartChunkTest, Implementations, );

TYPED_TEST(SmartChunkTest, ConstructedSmartChunkIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "9bfa7e75-b1bb-4811-8a79-df15d5375975");

    EXPECT_THAT(this->variation.sutProducer.get(), Eq(this->variation.chunkMock.sample()));

    EXPECT_THAT(this->variation.sutConsumer.get(), Eq(this->variation.chunkMock.sample()));
}

TYPED_TEST(SmartChunkTest, SmartChunkIsInvalidatedAfterMoveConstruction)
{
    ::testing::Test::RecordProperty("TEST_ID", "90c8db15-6cf2-4dfc-a9ca-cb1333081fe3");

    typename TestFixture::SutProducerType producer(std::move(this->variation.sutProducer));
    EXPECT_FALSE(this->variation.sutProducer);
    EXPECT_THAT(producer.get(), Eq(this->variation.chunkMock.sample()));

    typename TestFixture::SutConsumerType consumer(std::move(this->variation.sutConsumer));
    EXPECT_FALSE(this->variation.sutConsumer);
    EXPECT_THAT(consumer.get(), Eq(this->variation.chunkMock.sample()));
}

TYPED_TEST(SmartChunkTest, SmartChunkIsInvalidatedAfterMoveAssignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "f14523c6-c7ad-44b9-9a42-b1be652b28d0");

    this->variation.sutProducerForMove = std::move(this->variation.sutProducer);
    EXPECT_FALSE(this->variation.sutProducer);
    EXPECT_TRUE(this->variation.sutProducerForMove);
    EXPECT_FALSE(this->variation.sutProducer);
    EXPECT_THAT(this->variation.sutProducerForMove.get(), Eq(this->variation.chunkMock.sample()));

    this->variation.sutConsumerForMove = std::move(this->variation.sutConsumer);
    EXPECT_FALSE(this->variation.sutConsumer);
    EXPECT_TRUE(this->variation.sutConsumerForMove);
    EXPECT_FALSE(this->variation.sutConsumer);
    EXPECT_THAT(this->variation.sutConsumerForMove.get(), Eq(this->variation.chunkMock.sample()));
}

TYPED_TEST(SmartChunkTest, GetChunkHeaderWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "3364ddcd-5d38-4eec-9a26-335b1ce1a4b0");

    EXPECT_THAT(this->variation.sutProducer.getChunkHeader(), Eq(this->variation.chunkMock.chunkHeader()));
    const auto& constSutProducer = this->variation.sutProducer;
    EXPECT_THAT(constSutProducer.getChunkHeader(), Eq(this->variation.chunkMock.chunkHeader()));

    EXPECT_THAT(this->variation.sutConsumer.getChunkHeader(), Eq(this->variation.chunkMock.chunkHeader()));
    const auto& constSutConsumer = this->variation.sutConsumer;
    EXPECT_THAT(constSutConsumer.getChunkHeader(), Eq(this->variation.chunkMock.chunkHeader()));
}

TYPED_TEST(SmartChunkTest, PayloadAccessWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2ac7e40-6240-4f24-a3fc-be9c5ce51c8f");

    EXPECT_THAT(this->variation.sutProducer.get(), Eq(this->variation.chunkMock.sample()));
    const auto& constSutProducer = this->variation.sutProducer;
    EXPECT_THAT(constSutProducer.get(), Eq(this->variation.chunkMock.sample()));

    EXPECT_THAT(this->variation.sutConsumer.get(), Eq(this->variation.chunkMock.sample()));
    const auto& constSutConsumer = this->variation.sutConsumer;
    EXPECT_THAT(constSutConsumer.get(), Eq(this->variation.chunkMock.sample()));
}

TYPED_TEST(SmartChunkTest, MemberAccessWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "23fb13cd-1d3d-4e11-901f-68cf6a9d83d1");

    EXPECT_THAT(this->variation.sutProducer->val, Eq(EXPECTED_DATA_VALUE));
    const auto& constSutProducer = this->variation.sutProducer;
    EXPECT_THAT(constSutProducer->val, Eq(EXPECTED_DATA_VALUE));

    EXPECT_THAT(this->variation.sutConsumer->val, Eq(EXPECTED_DATA_VALUE));
    const auto& constSutConsumer = this->variation.sutConsumer;
    EXPECT_THAT(constSutConsumer->val, Eq(EXPECTED_DATA_VALUE));
}

TYPED_TEST(SmartChunkTest, DereferencingWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d43a5fe-3d42-4810-8730-e88c37c51f50");

    EXPECT_THAT((*this->variation.sutProducer).val, Eq(EXPECTED_DATA_VALUE));
    const auto& constSutProducer = this->variation.sutProducer;
    EXPECT_THAT((*constSutProducer).val, Eq(EXPECTED_DATA_VALUE));

    EXPECT_THAT((*this->variation.sutConsumer).val, Eq(EXPECTED_DATA_VALUE));
    const auto& constSutConsumer = this->variation.sutConsumer;
    EXPECT_THAT((*constSutConsumer).val, Eq(EXPECTED_DATA_VALUE));
}

} // namespace
