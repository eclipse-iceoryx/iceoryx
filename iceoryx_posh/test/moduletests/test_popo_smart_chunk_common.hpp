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

#include "iceoryx_posh/internal/popo/rpc_interface.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/request.hpp"
#include "iceoryx_posh/popo/response.hpp"
#include "iceoryx_posh/popo/sample.hpp"

namespace iox_test_popo_smart_chunk
{
using namespace iox::popo;

struct DummyData
{
    DummyData() = default;
    uint64_t val{42};
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
    MockPublisherInterface mockInterface;
    ChunkMock<DummyData, DummyHeader> chunkMock;
    SampleProducerType sutProducer{iox::cxx::unique_ptr<DummyData>(chunkMock.sample(), [](DummyData*) {}),
                                   mockInterface};
    SampleConsumerType sutConsumer{iox::cxx::unique_ptr<const DummyData>(chunkMock.sample(), [](const DummyData*) {})};
};

using RequestProducerType = Request<DummyData>;
using RequestConsumerType = Request<const DummyData>;

class MockRequestInterface : public RpcInterface<RequestProducerType, ClientSendError>
{
  public:
    iox::cxx::expected<ClientSendError> send(RequestProducerType&& request) noexcept override
    {
        auto req = std::move(request); // this step is necessary since the mock method doesn't execute the move
        return mockSend(std::move(req));
    }

    MOCK_METHOD(iox::cxx::expected<ClientSendError>, mockSend, (RequestProducerType &&), (noexcept));
};

class RequestTestCase
{
  public:
    MockRequestInterface mockInterface;
    ChunkMock<DummyData, RequestHeader> chunkMock;
    RequestProducerType sutProducer{iox::cxx::unique_ptr<DummyData>(chunkMock.sample(), [](DummyData*) {}),
                                    mockInterface};
    RequestConsumerType sutConsumer{iox::cxx::unique_ptr<const DummyData>(chunkMock.sample(), [](const DummyData*) {})};
};

using ResponseProducerType = Response<DummyData>;
using ResponseConsumerType = Response<const DummyData>;

class MockResponseInterface : public RpcInterface<ResponseProducerType, ServerSendError>
{
  public:
    iox::cxx::expected<ServerSendError> send(ResponseProducerType&& response) noexcept override
    {
        auto res = std::move(response); // this step is necessary since the mock method doesn't execute the move
        return mockSend(std::move(res));
    }

    MOCK_METHOD(iox::cxx::expected<ServerSendError>, mockSend, (ResponseProducerType &&), (noexcept));
};

class ResponseTestCase
{
  public:
    MockResponseInterface mockInterface;
    ChunkMock<DummyData, ResponseHeader> chunkMock;
    ResponseProducerType sutProducer{iox::cxx::unique_ptr<DummyData>(chunkMock.sample(), [](DummyData*) {}),
                                     mockInterface};
    ResponseConsumerType sutConsumer{
        iox::cxx::unique_ptr<const DummyData>(chunkMock.sample(), [](const DummyData*) {})};
};

} // namespace iox_test_popo_smart_chunk

#endif // IOX_TEST_POPO_SERVER_PORT_COMMON_HPP
