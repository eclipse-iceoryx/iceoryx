#ifndef IOX_DDS_GATEWAY_TEST_GOOGLE_MOCKS_HPP
#define IOX_DDS_GATEWAY_TEST_GOOGLE_MOCKS_HPP

#include "test.hpp"
#include "iceoryx_dds/dds/data_writer.hpp"
#include "iceoryx_dds/gateway/channel.hpp"
#include "iceoryx_dds/gateway/dds_gateway_generic.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"

using namespace ::testing;
using ::testing::_;

class MockGenericGateway
{
  public:
    MockGenericGateway(const iox::capro::Interfaces i){};
    MOCK_METHOD1(getCaProMessage, bool(iox::capro::CaproMessage&));
};

class MockSubscriber
{
  public:
    MockSubscriber(const iox::capro::ServiceDescription& sd){};
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

class MockDataWriter : public iox::dds::DataWriter<MockDataWriter>
{
  public:
    MockDataWriter(const iox::capro::ServiceDescription& sd){};
    MOCK_METHOD0(connect, void(void));
    MOCK_METHOD2(write, bool(uint8_t*, uint64_t));
    MOCK_CONST_METHOD0(getServiceId, std::string(void));
    MOCK_CONST_METHOD0(getInstanceId, std::string(void));
    MOCK_CONST_METHOD0(getEventId, std::string(void));
};

class MockGenericDDSGateway
{
  public:
    MockGenericDDSGateway(){};
    MockGenericDDSGateway(const iox::capro::Interfaces i){};
    MOCK_METHOD1(getCaProMessage, bool(iox::capro::CaproMessage&));
    MOCK_METHOD1(addChannel, iox::cxx::expected<iox::dds::Channel<MockSubscriber, MockDataWriter>, iox::dds::GatewayError>(const iox::capro::ServiceDescription&));
    MOCK_METHOD1(discardChannel, void(const iox::capro::ServiceDescription&));
    MOCK_METHOD1(
        findChannel,
        iox::cxx::optional<iox::dds::Channel<MockSubscriber, MockDataWriter>>(const iox::capro::ServiceDescription&));
    MOCK_METHOD1(forEachChannel, void(const iox::cxx::function_ref<void(iox::dds::Channel<MockSubscriber, MockDataWriter>&)>));
};

#endif // IOX_DDS_GATEWAY_TEST_GOOGLE_MOCKS_HPP
