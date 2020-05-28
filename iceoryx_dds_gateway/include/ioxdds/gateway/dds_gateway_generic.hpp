#pragma once

#include <iceoryx_posh/popo/gateway_generic.hpp>
#include <iceoryx_posh/capro/service_description.hpp>
#include <iceoryx_utils/cxx/vector.hpp>
#include <iceoryx_utils/internal/concurrent/smart_lock.hpp>

#include "ioxdds/dds/dds_configs.hpp"
namespace iox {
namespace dds {

template <typename channel_t>
class DDSGatewayGeneric : public iox::popo::GatewayGeneric
{

    using ChannelFactory = std::function<channel_t(const iox::capro::ServiceDescription)>;
    using ChannelVector = iox::cxx::vector<channel_t, MAX_CHANNEL_NUMBER>;
    using ConcurrentChannelVector = iox::concurrent::smart_lock<ChannelVector>;

protected:

    DDSGatewayGeneric();

    void loadConfiguration() noexcept;
    channel_t setupChannel(const iox::capro::ServiceDescription& service) noexcept;
    void discardChannel(const iox::capro::ServiceDescription& service) noexcept;

private:

    ChannelFactory m_channelFactory;
    ConcurrentChannelVector m_channels;

};

} // namespace dds
} // namespace iox

#include "ioxdds/internal/gateway/dds_gateway_generic.inl"
