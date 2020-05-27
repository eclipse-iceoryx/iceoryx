#include "ioxdds/gateway/base_gateway.hpp"

template<typename channel_t>
iox::dds::DDSGateway<channel_t>::DDSGateway() : iox::popo::GatewayGeneric(iox::capro::Interfaces::DDS)
{

}
