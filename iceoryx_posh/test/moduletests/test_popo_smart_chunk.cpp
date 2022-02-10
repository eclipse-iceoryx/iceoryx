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
    using SutType = typename T::SutType;
    using InterfaceType = typename T::InterfaceType;

    static constexpr void (SutType::*const SEND)() = T::SEND;
    static constexpr HeaderType& (SutType::*const GET_HEADER)() = T::GET_HEADER;
    static constexpr const HeaderType& (SutType::*const CONST_GET_HEADER)() const = T::CONST_GET_HEADER;

    SmartChunkTest()
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

  protected:
    ChunkMock<DataType, HeaderType> chunk;
    iox::cxx::unique_ptr<DataType> smartChunkPtr{this->chunk.sample(), [](DataType*) {}};
    InterfaceType mockInterface{};
    iox::cxx::optional<SutType> sut;
};

struct SampleTestCase
{
    using DataType = DummyData;
    using HeaderType = DummyHeader;
    using InterfaceType = MockInterface<iox::popo::PublisherInterface, DummyData, HeaderType>;
    using SutType = iox::popo::Sample<DummyData, HeaderType>;

    static constexpr void (SutType::*const SEND)() = &SutType::publish;
    static constexpr HeaderType& (SutType::*const GET_HEADER)() = &SutType::getUserHeader;
    static constexpr const HeaderType& (SutType::*const CONST_GET_HEADER)() const = &SutType::getUserHeader;
};

#if 0
struct RequestTestCase
{
    using DataType = DummyData;
    using HeaderType = iox::popo::RequestHeader;
    using InterfaceType = MockInterface<iox::popo::RequestInterface, DummyData, HeaderType>;
    using SutType = iox::popo::Request<DummyData>;

    static constexpr void (SutType::*const SEND)() = &SutType::send;
    static constexpr HeaderType& (SutType::*const GET_HEADER)() = &SutType::getRequestHeader;
    static constexpr const HeaderType& (SutType::*const CONST_GET_HEADER)() const = &SutType::getRequestHeader;
};

struct ResponseTestCase
{
    using DataType = DummyData;
    using HeaderType = iox::popo::RequestHeader;
    using InterfaceType = MockInterface<iox::popo::RequestInterface, DummyData, HeaderType>;
    using SutType = iox::popo::Request<DummyData>;

    static constexpr void (SutType::*const SEND)() = &SutType::send;
    static constexpr HeaderType& (SutType::*const GET_HEADER)() = &SutType::getRequestHeader;
    static constexpr const HeaderType& (SutType::*const CONST_GET_HEADER)() const = &SutType::getRequestHeader;
};

using Implementations = Types<SampleTestCase, RequestTestCase, ResponseTestCase>;
#endif

using Implementations = Types<SampleTestCase>;

TYPED_TEST_SUITE(SmartChunkTest, Implementations);

TYPED_TEST(SmartChunkTest, PublishesSmartChunkViaPublisherInterfaceWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b13578c-d654-4802-80a4-f45e5fd73268");

    this->sut.emplace(std::move(this->smartChunkPtr), this->mockInterface);

    EXPECT_CALL(this->mockInterface, publishMock).Times(1);

    (*this->sut.*TestFixture::SEND)();
}


TYPED_TEST(SmartChunkTest, PublishingEmptySmartChunkCallsErrorHandler)
{
    ::testing::Test::RecordProperty("TEST_ID", "b49bdcb3-6f8a-42c1-bb6c-a745f1a49b0e");
    this->sut.emplace(std::move(this->smartChunkPtr), this->mockInterface);

    EXPECT_CALL(this->mockInterface, publishMock).Times(1);
    (*this->sut.*TestFixture::SEND)();

    iox::cxx::optional<iox::Error> detectedError;
    auto errorHandlerGuard = iox::ErrorHandler::setTemporaryErrorHandler(
        [&detectedError](const iox::Error error, const std::function<void()>&, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::MODERATE));
        });
    (*this->sut.*TestFixture::SEND)();

    ASSERT_TRUE(detectedError.has_value());
    ASSERT_THAT(detectedError.value(), Eq(iox::Error::kPOSH__PUBLISHING_EMPTY_SAMPLE));
}

TYPED_TEST(SmartChunkTest, CallingGetUserHeaderFromNonConstTypeReturnsCorrectAddress)
{
    ::testing::Test::RecordProperty("TEST_ID", "d26dd24c-0c84-4c3d-96ab-bf51e52c9e9f");
    this->sut.emplace(std::move(this->smartChunkPtr), this->mockInterface);

    auto& header = (*this->sut.*TestFixture::GET_HEADER)();

    ASSERT_EQ(&header, this->chunk.userHeader());
}

TYPED_TEST(SmartChunkTest, CallingGetUserHeaderFromConstTypeReturnsCorrectAddress)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb6b6706-7a15-4ae2-a77a-d4b21431ca57");
    using SutType = typename TestFixture::SutType;

    this->sut.emplace(std::move(this->smartChunkPtr), this->mockInterface);

    const auto& header = (const_cast<const SutType&>(*this->sut).*TestFixture::CONST_GET_HEADER)();

    ASSERT_EQ(&header, this->chunk.userHeader());
}
} // namespace
