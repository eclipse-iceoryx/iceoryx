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

#ifndef IOX_TEST_POPO_SMART_CHUNK_COMMON_HPP
#define IOX_TEST_POPO_SMART_CHUNK_COMMON_HPP

#include "iceoryx_posh/internal/popo/publisher_interface.hpp"
#include "iceoryx_posh/internal/popo/rpc_interface.hpp"
#include "iceoryx_posh/internal/popo/smart_chunk.hpp"
#include "iceoryx_posh/popo/request.hpp"
#include "iceoryx_posh/popo/response.hpp"
#include "iceoryx_posh/popo/sample.hpp"
#include "iceoryx_posh/testing/mocks/chunk_mock.hpp"
#include "iox/unique_ptr.hpp"

namespace test_smart_chunk_common
{
using namespace iox::popo;

constexpr uint64_t EXPECTED_DATA_VALUE{42U};

struct DummyData
{
    DummyData() = default;
    uint64_t val{0U};
};
struct DummyHeader
{
    DummyHeader() = default;
    uint64_t counter = 0;
};

using SampleProducerType = Sample<DummyData, DummyHeader>;
using SampleConsumerType = Sample<const DummyData, const DummyHeader>;

class MockPublisherInterface : public PublisherInterface<DummyData, DummyHeader>
{
  public:
    void publish(SampleProducerType&& sample) noexcept override
    {
        auto s = std::move(sample); // this step is necessary since the mock method doesn't execute the move
        return mockSend(std::move(s));
    }

    MOCK_METHOD(void, mockSend, (SampleProducerType &&), (noexcept));
};

struct SampleTestCase
{
    using ProducerType = SampleProducerType;
    using ConsumerType = SampleConsumerType;

    SampleTestCase()
    {
        chunkMock.sample()->val = EXPECTED_DATA_VALUE;
        chunkMockForMove.sample()->val = 0U;
    }

    MockPublisherInterface mockInterface;
    ChunkMock<DummyData, DummyHeader> chunkMock;
    ChunkMock<DummyData, DummyHeader> chunkMockForMove;
    ProducerType sutProducer{iox::unique_ptr<DummyData>(chunkMock.sample(), [](DummyData*) {}), mockInterface};
    ProducerType sutProducerForMove{iox::unique_ptr<DummyData>(chunkMockForMove.sample(), [](DummyData*) {}),
                                    mockInterface};
    ConsumerType sutConsumer{iox::unique_ptr<const DummyData>(chunkMock.sample(), [](const DummyData*) {})};
    ConsumerType sutConsumerForMove{
        iox::unique_ptr<const DummyData>(chunkMockForMove.sample(), [](const DummyData*) {})};
};

using RequestProducerType = Request<DummyData>;
using RequestConsumerType = Request<const DummyData>;

class MockRequestInterface : public RpcInterface<RequestProducerType, ClientSendError>
{
  public:
    iox::expected<void, ClientSendError> send(RequestProducerType&& request) noexcept override
    {
        auto req = std::move(request); // this step is necessary since the mock method doesn't execute the move
        return mockSend(std::move(req));
    }

    MOCK_METHOD((iox::expected<void, ClientSendError>), mockSend, (RequestProducerType &&), (noexcept));
};

class RequestTestCase
{
  public:
    using ProducerType = RequestProducerType;
    using ConsumerType = RequestConsumerType;

    RequestTestCase()
    {
        chunkMock.sample()->val = EXPECTED_DATA_VALUE;
        chunkMockForMove.sample()->val = 0U;
    }

    MockRequestInterface mockInterface;
    ChunkMock<DummyData, RequestHeader> chunkMock;
    ChunkMock<DummyData, RequestHeader> chunkMockForMove;
    ProducerType sutProducer{iox::unique_ptr<DummyData>(chunkMock.sample(), [](DummyData*) {}), mockInterface};
    ProducerType sutProducerForMove{iox::unique_ptr<DummyData>(chunkMockForMove.sample(), [](DummyData*) {}),
                                    mockInterface};
    ConsumerType sutConsumer{iox::unique_ptr<const DummyData>(chunkMock.sample(), [](const DummyData*) {})};
    ConsumerType sutConsumerForMove{
        iox::unique_ptr<const DummyData>(chunkMockForMove.sample(), [](const DummyData*) {})};
};

using ResponseProducerType = Response<DummyData>;
using ResponseConsumerType = Response<const DummyData>;

class MockResponseInterface : public RpcInterface<ResponseProducerType, ServerSendError>
{
  public:
    iox::expected<void, ServerSendError> send(ResponseProducerType&& response) noexcept override
    {
        auto res = std::move(response); // this step is necessary since the mock method doesn't execute the move
        return mockSend(std::move(res));
    }

    MOCK_METHOD((iox::expected<void, ServerSendError>), mockSend, (ResponseProducerType &&), (noexcept));
};

class ResponseTestCase
{
  public:
    using ProducerType = ResponseProducerType;
    using ConsumerType = ResponseConsumerType;

    ResponseTestCase()
    {
        chunkMock.sample()->val = EXPECTED_DATA_VALUE;
        chunkMockForMove.sample()->val = 0U;
    }

    MockResponseInterface mockInterface;
    ChunkMock<DummyData, ResponseHeader> chunkMock;
    ChunkMock<DummyData, ResponseHeader> chunkMockForMove;
    ProducerType sutProducer{iox::unique_ptr<DummyData>(chunkMock.sample(), [](DummyData*) {}), mockInterface};
    ProducerType sutProducerForMove{iox::unique_ptr<DummyData>(chunkMockForMove.sample(), [](DummyData*) {}),
                                    mockInterface};
    ConsumerType sutConsumer{iox::unique_ptr<const DummyData>(chunkMock.sample(), [](const DummyData*) {})};
    ConsumerType sutConsumerForMove{
        iox::unique_ptr<const DummyData>(chunkMockForMove.sample(), [](const DummyData*) {})};
};

} // namespace test_smart_chunk_common

#endif // IOX_TEST_POPO_SERVER_PORT_COMMON_HPP
