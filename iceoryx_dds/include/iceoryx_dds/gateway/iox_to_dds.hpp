#pragma once

#include <iceoryx_posh/popo/subscriber.hpp>

#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/gateway/dds_gateway_generic.hpp"
#include "iceoryx_dds/gateway/channel.hpp"

namespace iox {
namespace dds {

template <typename channel_t = iox::dds::Channel<iox::popo::Subscriber, iox::dds::data_writer_t>>
class Iceoryx2DDSGateway : public iox::dds::DDSGatewayGeneric<channel_t>
{
    using ChannelFactory = std::function<channel_t(const iox::capro::ServiceDescription)>;
public:
    Iceoryx2DDSGateway() noexcept;
    Iceoryx2DDSGateway(ChannelFactory channelFactory) noexcept;
    void loadConfiguration(GatewayConfig config);
    void discover(const iox::capro::CaproMessage& msg) noexcept;
    void forward() noexcept;
};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/gateway/iox_to_dds.inl"
