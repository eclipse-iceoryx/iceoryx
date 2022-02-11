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
    auto& getHeader(SutType& sut)
    {
        return T::getHeader(sut);
    }

    template <typename SutType>
    auto& getConstHeader(const SutType& sut)
    {
        return T::getHeader(sut);
    }

    template <typename T1>
    const T1& makeConst(T1& t)
    {
        return const_cast<const T1&>(t);
    }

    template <typename T1>
    void verifyNotEmpty(T1& helper)
    {
        ASSERT_TRUE(helper.sut);
        ASSERT_THAT(helper.sut.get(), Ne(nullptr));
        ASSERT_THAT(helper.sut.operator->(), Ne(nullptr));
        ASSERT_THAT(makeConst(helper.sut).get(), Ne(nullptr));
        ASSERT_THAT(makeConst(helper.sut).operator->(), Ne(nullptr));
        ASSERT_THAT(helper.sut.getChunkHeader(), Ne(nullptr));
        ASSERT_THAT(makeConst(helper.sut).getChunkHeader(), Ne(nullptr));
    }

    template <typename T1>
    void verifyContent(T1& helper, const uint32_t dataValue, const uint64_t headerValue)
    {
        verifyNotEmpty(helper);

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

        SutHelper(SutType<Data, Header>&& origin)
            : sut{std::move(origin)}
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
    using InterfaceType = MockInterface<iox::popo::PublisherInterface, DataType, HeaderType>;

    template <typename Data, typename Header>
    using SutType = iox::popo::Sample<Data, Header>;

    template <typename SutType>
    static void send(SutType& sut)
    {
        sut.publish();
    }

    template <typename SutType>
    static auto& getHeader(SutType& sut)
    {
        return sut.getUserHeader();
    }

    template <typename SutType>
    static auto& getHeader(const SutType& sut)
    {
        return sut.getUserHeader();
    }
};


using Implementations = Types<SampleTestCase>;

TYPED_TEST_SUITE(SmartChunkTest, Implementations);

TYPED_TEST(SmartChunkTest, ProducerConstructedSmartChunkIsValid)
{
    constexpr uint32_t DATA_VALUE = 123;
    constexpr uint64_t HEADER_VALUE = 456;

    this->setUnderlyingData(this->producer, DATA_VALUE, HEADER_VALUE);
    this->verifyContent(this->producer, DATA_VALUE, HEADER_VALUE);
}

TYPED_TEST(SmartChunkTest, ConsumerConstructedSmartChunkIsValid)
{
    constexpr uint32_t DATA_VALUE = 789;
    constexpr uint64_t HEADER_VALUE = 1337;

    this->setUnderlyingData(this->consumer, DATA_VALUE, HEADER_VALUE);
    this->verifyContent(this->consumer, DATA_VALUE, HEADER_VALUE);
}

TYPED_TEST(SmartChunkTest, ProducerSmartChunkIsInvalidatedAfterMoveConstruction)
{
    constexpr uint32_t DATA_VALUE = 12301;
    constexpr uint64_t HEADER_VALUE = 9817238;

    this->setUnderlyingData(this->producer, DATA_VALUE, HEADER_VALUE);

    typename TestFixture::ProducerHelper destination(std::move(this->producer.sut));

    this->verifyEmpty(this->producer);
    this->verifyContent(destination, DATA_VALUE, HEADER_VALUE);
}

TYPED_TEST(SmartChunkTest, ConsumerSmartChunkIsInvalidatedAfterMoveConstruction)
{
    constexpr uint32_t DATA_VALUE = 88121;
    constexpr uint64_t HEADER_VALUE = 55123;

    this->setUnderlyingData(this->consumer, DATA_VALUE, HEADER_VALUE);

    typename TestFixture::ConsumerHelper destination(std::move(this->consumer.sut));

    this->verifyEmpty(this->consumer);
    this->verifyContent(destination, DATA_VALUE, HEADER_VALUE);
}

TYPED_TEST(SmartChunkTest, ProducerSmartChunkIsInvalidatedAfterMoveAssignment)
{
    constexpr uint32_t DATA_VALUE = 8812165;
    constexpr uint64_t HEADER_VALUE = 55123123;

    this->setUnderlyingData(this->producer, DATA_VALUE, HEADER_VALUE);

    typename TestFixture::ProducerHelper destination(this->mockInterface);

    destination.sut = std::move(this->producer.sut);

    this->verifyEmpty(this->producer);
    this->verifyContent(destination, DATA_VALUE, HEADER_VALUE);
}

TYPED_TEST(SmartChunkTest, ConsumerSmartChunkIsInvalidatedAfterMoveAssignment)
{
    constexpr uint32_t DATA_VALUE = 8165;
    constexpr uint64_t HEADER_VALUE = 1123;

    this->setUnderlyingData(this->consumer, DATA_VALUE, HEADER_VALUE);

    typename TestFixture::ConsumerHelper destination;

    destination.sut = std::move(this->consumer.sut);

    this->verifyEmpty(this->consumer);
    this->verifyContent(destination, DATA_VALUE, HEADER_VALUE);
}

TYPED_TEST(SmartChunkTest, SendingSmartChunkSucceeds)
{
    EXPECT_CALL(this->mockInterface, publishMock).Times(1);

    this->send(this->producer.sut);
    this->verifyEmpty(this->producer);
}

TYPED_TEST(SmartChunkTest, SendingSmartChunkMultipleTimesFails)
{
    EXPECT_CALL(this->mockInterface, publishMock).Times(1);

    this->send(this->producer.sut);

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::setTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>&, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::MODERATE));
        });
    this->send(this->producer.sut);

    ASSERT_TRUE(detectedError.has_value());
    ASSERT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__PUBLISHING_EMPTY_SAMPLE));
}

TYPED_TEST(SmartChunkTest, SendingMovedSmartChunkFails)
{
    typename TestFixture::ProducerHelper destination(this->mockInterface);

    destination.sut = std::move(this->producer.sut);

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::setTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>&, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::MODERATE));
        });
    this->send(this->producer.sut);

    ASSERT_TRUE(detectedError.has_value());
    ASSERT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__PUBLISHING_EMPTY_SAMPLE));
}

TYPED_TEST(SmartChunkTest, SendingDestinationOfValidMoveOriginSucceeds)
{
    typename TestFixture::ProducerHelper destination(this->mockInterface);

    destination.sut = std::move(this->producer.sut);

    EXPECT_CALL(this->mockInterface, publishMock).Times(1);
    this->send(destination.sut);
}
} // namespace
