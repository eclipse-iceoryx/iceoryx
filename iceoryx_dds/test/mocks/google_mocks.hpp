// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_DDS_GATEWAY_TEST_GOOGLE_MOCKS_HPP
#define IOX_DDS_GATEWAY_TEST_GOOGLE_MOCKS_HPP

#include "iceoryx_dds/dds/data_reader.hpp"
#include "iceoryx_dds/dds/data_writer.hpp"
#include "iceoryx_posh/gateway/channel.hpp"
#include "iceoryx_posh/gateway/gateway_generic.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

class MockPublisher
{
  public:
    MockPublisher(const iox::capro::ServiceDescription&){};
    MOCK_METHOD0(offer, void(void));
    MOCK_METHOD1(allocateChunk, void*(uint32_t));
    MOCK_METHOD1(sendChunk, void(const void* const));
};

class MockSubscriber
{
  public:
    MockSubscriber(const iox::capro::ServiceDescription&){};
    MOCK_METHOD0(isDestroyed, void()); // Allows testing for destruction
    virtual ~MockSubscriber()
    {
        isDestroyed();
    }
    MOCK_CONST_METHOD0(hasNewChunks, bool());
    MOCK_METHOD0(getServiceDescription, iox::capro::ServiceDescription());
    MOCK_METHOD1(getChunk, bool(const iox::mepoo::ChunkHeader**));
    MOCK_METHOD1(releaseChunk, bool(const void* const payload));
    MOCK_METHOD1(subscribe, void(const uint32_t));
};

class MockDataReader
{
  public:
    MockDataReader(const iox::capro::ServiceDescription&){};
    MOCK_METHOD0(connect, void(void));
    MOCK_METHOD0(peekNextSize, iox::cxx::optional<uint64_t>(void));
    MOCK_METHOD2(takeNext, iox::cxx::expected<iox::dds::DataReaderError>(uint8_t* const, const uint64_t&));
    MOCK_METHOD3(take,
                 iox::cxx::expected<uint64_t, iox::dds::DataReaderError>(uint8_t* const buffer,
                                                                         const uint64_t&,
                                                                         const iox::cxx::optional<uint64_t>&));
    MOCK_CONST_METHOD0(getServiceId, std::string(void));
    MOCK_CONST_METHOD0(getInstanceId, std::string(void));
    MOCK_CONST_METHOD0(getEventId, std::string(void));
};

class MockDataWriter
{
  public:
    MockDataWriter(const iox::capro::ServiceDescription&){};
    MOCK_METHOD0(connect, void(void));
    MOCK_METHOD2(write, bool(uint8_t*, uint64_t));
    MOCK_CONST_METHOD0(getServiceId, std::string(void));
    MOCK_CONST_METHOD0(getInstanceId, std::string(void));
    MOCK_CONST_METHOD0(getEventId, std::string(void));
};

template <typename channel_t>
class MockGenericGateway
{
  public:
    MockGenericGateway(){};
    MockGenericGateway(const iox::capro::Interfaces, iox::units::Duration, iox::units::Duration){};
    MOCK_METHOD1(getCaProMessage, bool(iox::capro::CaproMessage&));
    MOCK_METHOD1_T(addChannel,
                   iox::cxx::expected<channel_t, iox::gw::GatewayError>(const iox::capro::ServiceDescription&));
    MOCK_METHOD1(discardChannel, iox::cxx::expected<iox::gw::GatewayError>(const iox::capro::ServiceDescription&));
    MOCK_METHOD1_T(findChannel, iox::cxx::optional<channel_t>(const iox::capro::ServiceDescription&));
    MOCK_METHOD1_T(forEachChannel, void(const iox::cxx::function_ref<void(channel_t&)>));
};

#endif // IOX_DDS_GATEWAY_TEST_GOOGLE_MOCKS_HPP
