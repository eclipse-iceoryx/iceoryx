
#include "ioxdds/gateway/dds_to_iox.hpp"

namespace iox {
namespace dds {

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline DDS2IceoryxGateway<gateway_t, subscriber_t, data_writer_t>::DDS2IceoryxGateway()
    : gateway_t(iox::capro::Interfaces::DDS)
{
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline DDS2IceoryxGateway<gateway_t, subscriber_t, data_writer_t>::~DDS2IceoryxGateway()
{
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void DDS2IceoryxGateway<gateway_t, subscriber_t, data_writer_t>::runMultithreaded() noexcept
{
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void DDS2IceoryxGateway<gateway_t, subscriber_t, data_writer_t>::discoveryLoop() noexcept
{

}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void DDS2IceoryxGateway<gateway_t, subscriber_t, data_writer_t>::forwardingLoop() noexcept
{

}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void DDS2IceoryxGateway<gateway_t, subscriber_t, data_writer_t>::shutdown() noexcept
{

}

} // namespace dds
} // namespace iox
