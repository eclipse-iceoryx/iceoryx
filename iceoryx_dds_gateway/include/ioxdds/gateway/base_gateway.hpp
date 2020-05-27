#pragma once

#include <iceoryx_posh/popo/gateway_generic.hpp>

namespace iox {
namespace dds {

template <typename channel_t>
class DDSGateway : public iox::popo::GatewayGeneric
{

protected:

    DDSGateway();

    void loadConfiguration() noexcept;
    channel_t setupChannel(const iox::capro::ServiceDescription& service) noexcept;
    void discardChannel(const iox::capro::ServiceDescription& service) noexcept;

};

} // namespace dds
} // namespace iox

#include "ioxdds/internal/gateway/base_gateway.inl"
