#pragma once

#include <iceoryx_posh/popo/publisher.hpp>

#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/gateway/dds_gateway_generic.hpp"
#include "iceoryx_dds/gateway/channel.hpp"

namespace iox {
namespace dds {

template <typename channel_t = iox::dds::Channel<iox::popo::Publisher, iox::dds::data_reader_t>>
class DDS2IceoryxGateway : public iox::dds::DDSGatewayGeneric<channel_t>
{
public:
    DDS2IceoryxGateway() noexcept;
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward() noexcept;
};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/gateway/dds_to_iox.inl"
