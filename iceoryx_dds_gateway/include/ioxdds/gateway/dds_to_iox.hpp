#pragma once

#include <iceoryx_posh/popo/gateway_generic.hpp>
#include <iceoryx_posh/popo/publisher.hpp>

#include "ioxdds/dds/dds_types.hpp";
namespace iox {
namespace dds {

template <typename gateway_t = iox::popo::GatewayGeneric,
          typename publisher_t = iox::popo::Publisher,
          typename data_reader_t = iox::dds::data_reader_t>
class DDS2IceoryxGateeway : gateway_t
{

};

} // namespace dds
} // namespace iox
