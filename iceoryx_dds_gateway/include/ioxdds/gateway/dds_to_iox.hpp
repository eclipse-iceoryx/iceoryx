#pragma once

#include <iceoryx_posh/popo/publisher.hpp>

#include "ioxdds/dds/dds_types.hpp"
#include "ioxdds/gateway/dds_gateway_generic.hpp"
#include "ioxdds/gateway/channel.hpp"
#include "ioxdds/gateway/input_channel.hpp"

namespace iox {
namespace dds {

template <typename publisher_t = iox::popo::Publisher,
          typename data_reader_t = iox::dds::data_reader_t>
class DDS2IceoryxGateway : public iox::dds::DDSGatewayGeneric<iox::dds::Channel<publisher_t, data_reader_t>>
{
public:
    DDS2IceoryxGateway();
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward() noexcept;
};

} // namespace dds
} // namespace iox

#include "ioxdds/internal/gateway/dds_to_iox.inl"
