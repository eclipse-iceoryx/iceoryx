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

#include "iceoryx_posh/popo/publisher.hpp"

#include "iceoryx_hoofs/cxx/unique_ptr.hpp"
#include "iceoryx_posh/internal/popo/smart_chunk.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/request.hpp"
#include "iceoryx_posh/popo/response.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using ::testing::_;

struct DummyData
{
    DummyData() = default;
    uint32_t val = 42;
};
struct DummyHeader
{
    DummyHeader() = default;
    uint64_t counter = 0;
};

template <template <typename, typename> class BaseType, typename T, typename H = iox::mepoo::NoUserHeader>
class MockInterface : public BaseType<T, H>
{
  public:
    using SampleType = iox::popo::Sample<T, H>;

    void publish(iox::popo::Sample<T, H>&& sample) noexcept override
    {
        auto s = std::move(sample); // this step is necessary since the mock method doesn't execute the move
        return publishMock(std::move(s));
    }
    MOCK_METHOD(void, publishMock, ((iox::popo::Sample<T, H> &&)), (noexcept));
};

template <typename T, typename H = iox::mepoo::NoUserHeader>
using MockPublisherInterface = MockInterface<iox::popo::PublisherInterface, T, H>;

template <typename T>
class SmartChunkTest : public Test
{
  public:
    using DataType = typename T::DataType;
    using HeaderType = typename T::HeaderType;
    template <typename Data, typename Header>
    using SutType = typename T::template SutType<Data, Header>;
    using InterfaceType = typename T::InterfaceType;

    template <typename SutType>
    void send(SutType& sut)
    {
        T::send(sut);
    }

    template <typename SutType>
    HeaderType& getHeader(SutType& sut)
    {
        return T::getHeader(sut);
    }

    template <typename SutType>
    const HeaderType& getConstHeader(const SutType& sut)
    {
        return T::getHeader(sut);
    }

    template <typename T1>
    const T1& makeConst(T1& t)
    {
        return const_cast<const T1&>(t);
    }

    template <typename T1>
    void verifyNotEmpty(T1& sut)
    {
        ASSERT_TRUE(sut);
        ASSERT_THAT(sut.get(), Ne(nullptr));
        ASSERT_THAT(sut.operator->(), Ne(nullptr));
        ASSERT_THAT(makeConst(sut).get(), Ne(nullptr));
        ASSERT_THAT(makeConst(sut).operator->(), Ne(nullptr));
        ASSERT_THAT(sut.getChunkHeader(), Ne(nullptr));
        ASSERT_THAT(makeConst(sut).getChunkHeader(), Ne(nullptr));
    }

    template <typename T1>
    void verifyContent(T1& helper, const uint32_t dataValue, const uint64_t headerValue)
    {
        verifyNotEmpty(helper.sut);

        EXPECT_THAT(helper.sut.get()->val, Eq(dataValue));
        EXPECT_THAT(makeConst(helper.sut).get()->val, Eq(dataValue));
        EXPECT_THAT(helper.sut->val, Eq(dataValue));
        EXPECT_THAT(makeConst(helper.sut)->val, Eq(dataValue));
        EXPECT_THAT((*helper.sut).val, Eq(dataValue));
        EXPECT_THAT((*makeConst(helper.sut)).val, Eq(dataValue));

        EXPECT_THAT(getHeader(helper.sut).counter, Eq(headerValue));
        EXPECT_THAT(getConstHeader(helper.sut).counter, Eq(headerValue));
    }

    template <typename T1>
    void verifyEmpty(T1& helper)
    {
        EXPECT_FALSE(helper.sut);
        EXPECT_THAT(helper.sut.get(), Eq(nullptr));
        EXPECT_THAT(helper.sut.operator->(), Eq(nullptr));
        EXPECT_THAT(makeConst(helper.sut).get(), Eq(nullptr));
        EXPECT_THAT(makeConst(helper.sut).operator->(), Eq(nullptr));
        EXPECT_THAT(helper.sut.getChunkHeader(), Eq(nullptr));
        EXPECT_THAT(makeConst(helper.sut).getChunkHeader(), Eq(nullptr));
    }

    template <typename T1>
    void setUnderlyingData(const T1& sut, const uint32_t dataValue, const uint64_t headerValue)
    {
        const_cast<uint32_t&>(sut.chunk.sample()->val) = dataValue;
        const_cast<uint64_t&>(sut.chunk.userHeader()->counter) = headerValue;
    }

  protected:
    InterfaceType mockInterface{};

    template <typename Data, typename Header>
    struct SutHelper
    {
        SutHelper()
            : sut{iox::cxx::unique_ptr<Data>(this->chunk.sample(), [](Data*) {})}
        {
        }

        SutHelper(InterfaceType& interface)
            : sut{iox::cxx::unique_ptr<Data>(this->chunk.sample(), [](Data*) {}), interface}
        {
        }

        ChunkMock<Data, Header> chunk;
        SutType<Data, Header> sut;
    };

    using ProducerHelper = SutHelper<DataType, HeaderType>;
    using ConsumerHelper = SutHelper<const DataType, const HeaderType>;

    ProducerHelper producer{this->mockInterface};
    ConsumerHelper consumer;
};

struct SampleTestCase
{
    using DataType = DummyData;
    using HeaderType = DummyHeader;
    using InterfaceType = MockInterface<iox::popo::PublisherInterface, DummyData, HeaderType>;

    template <typename Data, typename Header>
    using SutType = iox::popo::Sample<Data, Header>;

    template <typename SutType>
    static void send(SutType& sut)
    {
        sut.publish();
    }

    template <typename SutType>
    static HeaderType& getHeader(SutType& sut)
    {
        return sut.getUserHeader();
    }

    template <typename SutType>
    static const HeaderType& getHeader(const SutType& sut)
    {
        return sut.getUserHeader();
    }
};


using Implementations = Types<SampleTestCase>;

TYPED_TEST_SUITE(SmartChunkTest, Implementations);

TYPED_TEST(SmartChunkTest, ProducerConstructedSmartChunkIsValid)
{
    this->setUnderlyingData(this->producer, 123, 456);

    this->verifyContent(this->producer, 123, 456);
}

TYPED_TEST(SmartChunkTest, ConsumerConstructedSmartChunkIsValid)
{
    this->setUnderlyingData(this->consumer, 789, 1337);

    this->verifyNotEmpty(this->consumer.sut);
}

#if 0
TYPED_TEST(SmartChunkTest, ProducerSmartChunkIsInvalidatedAfterMoveConstruction)
{
    this->producer.sut->val = 1337;
    this->getHeader(this->producer.sut).counter = 73;

    auto moved{std::move(this->producer.sut)};

    verifyContent(moved, 1337, 73);

    verifyEmpty(this->producer.sut);
}

TYPED_TEST(SmartChunkTest, ConsumerSmartChunkIsInvalidatedAfterMoveConstruction)
{
    auto moved{std::move(this->consumer.sut)};

    verifyNotEmpty(moved);

    verifyEmpty(this->consumer.sut);
}

TYPED_TEST(SmartChunkTest, ProducerSmartChunkIsInvalidatedAfterMoveAssignment)
{
    typename TestFixture::ProducerHelper destination(this->mockInterface);
    this->producer.sut->val = 81921;
    this->getHeader(this->producer.sut).counter = 55551;

    destination.sut = std::move(this->producer.sut);

    EXPECT_TRUE(destination.sut);
    ASSERT_THAT(destination.sut.get(), Ne(nullptr));
    EXPECT_THAT(destination.sut.get()->val, Eq(81921));
    EXPECT_THAT(this->getHeader(destination.sut).counter, Eq(55551));

    verifyEmpty(this->producer.sut);
}

TYPED_TEST(SmartChunkTest, ConsumerSmartChunkIsInvalidatedAfterMoveAssignment)
{
    typename TestFixture::ConsumerHelper destination;
    destination.sut = std::move(this->consumer.sut);

    EXPECT_TRUE(destination.sut);

    verifyEmpty(this->consumer.sut);
}

TYPED_TEST(SmartChunkTest, PublishesSmartChunkViaPublisherInterfaceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b13578c-d654-4802-80a4-f45e5fd73268");

    this->sut.emplace(std::move(this->smartChunkPtr), this->mockInterface);

    EXPECT_CALL(this->mockInterface, publishMock).Times(1);

    this->send();
}


TYPED_TEST(SmartChunkTest, PublishingEmptySmartChunkCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "b49bdcb3-6f8a-42c1-bb6c-a745f1a49b0e");
    this->sut.emplace(std::move(this->smartChunkPtr), this->mockInterface);

    EXPECT_CALL(this->mockInterface, publishMock).Times(1);
    this->send();

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::setTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>&, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::MODERATE));
        });
    this->send();

    ASSERT_TRUE(detectedError.has_value());
    ASSERT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__PUBLISHING_EMPTY_SAMPLE));
}

TYPED_TEST(SmartChunkTest, CallingGetUserHeaderFromNonConstTypeReturnsCorrectAddress)
{
    ::testing::Test::RecordProperty("TEST_ID", "d26dd24c-0c84-4c3d-96ab-bf51e52c9e9f");
    this->sut.emplace(std::move(this->smartChunkPtr), this->mockInterface);

    auto& header = this->getHeader();

    ASSERT_EQ(&header, this->chunk.userHeader());
}

TYPED_TEST(SmartChunkTest, CallingGetUserHeaderFromConstTypeReturnsCorrectAddress)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb6b6706-7a15-4ae2-a77a-d4b21431ca57");

    this->sut.emplace(std::move(this->smartChunkPtr), this->mockInterface);

    const auto& header = this->getConstHeader();

    ASSERT_EQ(&header, this->chunk.userHeader());
}
#endif
} // namespace
