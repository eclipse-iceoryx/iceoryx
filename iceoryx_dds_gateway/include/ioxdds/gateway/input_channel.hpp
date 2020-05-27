#pragma once

#include <iceoryx_posh/popo/publisher.hpp>
#include <iceoryx_utils/internal/objectpool/objectpool.hpp>

#include "ioxdds/dds/dds_configs.hpp"
#include "ioxdds/dds/dds_types.hpp"
#include "ioxdds/dds/data_reader.hpp"

namespace iox {
namespace dds {

template <typename publisher_t = iox::popo::Publisher, typename data_reader_t = iox::dds::data_reader_t>
class InputChannel
{
  public:
    using PublisherPtr = std::shared_ptr<publisher_t>;
    using PublisherPool = iox::cxx::ObjectPool<publisher_t, MAX_CHANNEL_NUMBER>;
    using DataReaderPtr = std::shared_ptr<data_reader_t>;
    using DataReaderPool = iox::cxx::ObjectPool<data_reader_t, MAX_CHANNEL_NUMBER>;

    InputChannel(const iox::capro::ServiceDescription& service,
            const PublisherPtr publisher,
            const DataReaderPtr dataReader) noexcept;

    static InputChannel create(const iox::capro::ServiceDescription& service) noexcept;

    iox::capro::ServiceDescription getService() const noexcept;
    PublisherPtr getPublisher() const noexcept;
    DataReaderPtr getDataReader() const noexcept;

  private:
    // Store in data segment - too large to keep in stack.
    static PublisherPool s_publisherPool;
    static DataReaderPool s_dataReaderPool;

    iox::capro::ServiceDescription m_service;
    PublisherPtr m_publisher;
    DataReaderPtr m_dataReader;
};

} // dds
} // iox

#include "ioxdds/internal/gateway/input_channel.inl"
