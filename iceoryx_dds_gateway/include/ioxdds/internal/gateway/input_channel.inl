#include "ioxdds/gateway/input_channel.hpp"

namespace iox {
namespace dds {

// Typedefs
template <typename publisher_t>
using PublisherPool = iox::cxx::ObjectPool<publisher_t, MAX_CHANNEL_NUMBER>;
template <typename data_reader_t>
using DataReaderPool = iox::cxx::ObjectPool<data_reader_t, MAX_CHANNEL_NUMBER>;

// Statics
template <typename publisher_t, typename data_reader_t>
PublisherPool<publisher_t> InputChannel<publisher_t, data_reader_t>::s_publisherPool = PublisherPool();
template <typename publisher_t, typename data_reader_t>
DataReaderPool<data_reader_t> InputChannel<publisher_t, data_reader_t>::s_dataReaderPool = DataReaderPool();

template <typename publisher_t, typename data_reader_t>
inline InputChannel<publisher_t, data_reader_t>::InputChannel(const iox::capro::ServiceDescription& service,
                                                     const PublisherPtr publisher,
                                                     const DataReaderPtr dataReader) noexcept
    : m_service(service),
      m_publisher(publisher),
      m_dataReader(dataReader)
{
}

template <typename publisher_t, typename data_reader_t>
inline InputChannel<publisher_t, data_reader_t>
InputChannel<publisher_t, data_reader_t>::create(const iox::capro::ServiceDescription& service) noexcept
{
    // Create objects in the pool.
    auto rawPublisherPtr = s_publisherPool.create(service);
    auto rawDataReaderPtr = s_dataReaderPool.create(
        service.getServiceIDString(), service.getInstanceIDString(), service.getEventIDString());

    // Wrap in smart pointer with custom deleter to ensure automatic cleanup.
    auto publisherPtr = PublisherPtr(rawPublisherPtr, [](publisher_t* p) { s_publisherPool.free(p); });
    auto dataReaderPtr = DataReaderPtr(rawDataReaderPtr, [](data_reader_t* p) { s_dataReaderPool.free(p); });

    return InputChannel(service, publisherPtr, dataReaderPtr);
}

template <typename publisher_t, typename data_reader_t>
inline iox::capro::ServiceDescription InputChannel<publisher_t, data_reader_t>::getService() const noexcept
{
    return m_service;
}

template <typename publisher_t, typename data_reader_t>
inline std::shared_ptr<publisher_t> InputChannel<publisher_t, data_reader_t>::getPublisher() const noexcept
{
    return m_publisher;
}

template <typename publisher_t, typename data_reader_t>
inline std::shared_ptr<data_reader_t> InputChannel<publisher_t, data_reader_t>::getDataReader() const noexcept
{
    return m_dataReader;
}

}
}
