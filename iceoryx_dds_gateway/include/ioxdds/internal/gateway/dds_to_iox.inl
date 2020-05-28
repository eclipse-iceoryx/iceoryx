
#include <cpptoml.h>

#include <iceoryx_posh/capro/service_description.hpp>
#include <iceoryx_utils/cxx/string.hpp>

#include "ioxdds/gateway/dds_to_iox.hpp"
#include "ioxdds/internal/log/logging.hpp"

namespace iox {
namespace dds {

template <typename publisher_t, typename data_reader_t>
inline DDS2IceoryxGateway<publisher_t, data_reader_t>::DDS2IceoryxGateway()
{
    // Initialize pre-configured services
    this->loadConfiguration();
}

template <typename publisher_t, typename data_reader_t>
inline void
DDS2IceoryxGateway<publisher_t, data_reader_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{

}

template <typename publisher_t, typename data_reader_t>
inline void DDS2IceoryxGateway<publisher_t, data_reader_t>::forward() noexcept
{

}

} // namespace dds
} // namespace iox
