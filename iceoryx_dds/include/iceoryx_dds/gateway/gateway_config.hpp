#pragma once

#include <iceoryx_posh/capro/service_description.hpp>
#include <iceoryx_posh/iceoryx_posh_types.hpp>
#include <iceoryx_utils/cxx/vector.hpp>

namespace iox {
namespace dds {

struct GatewayConfig
{
public:
    iox::cxx::vector<iox::capro::ServiceDescription, iox::MAX_PORT_NUMBER> m_configuredServices;
};

}
}
