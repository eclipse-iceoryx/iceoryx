
#include <cpptoml.h>

#include <iceoryx_posh/capro/service_description.hpp>
#include <iceoryx_utils/cxx/string.hpp>

#include "iceoryx_dds/gateway/dds_to_iox.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"

namespace iox {
namespace dds {

template <typename channel_t>
inline DDS2IceoryxGateway<channel_t>::DDS2IceoryxGateway() noexcept
    : iox::dds::DDSGatewayGeneric<channel_t>()
{
}

template<typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::loadConfiguration(GatewayConfig config) noexcept
{
}


template <typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
}

template <typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::forward() noexcept
{
}

} // namespace dds
} // namespace iox
