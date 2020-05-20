#pragma once

#include <iceoryx_posh/iceoryx_posh_types.hpp>

namespace iox
{
namespace dds
{
static constexpr units::Duration DISCOVERY_PERIOD = 1000_ms;
static constexpr units::Duration FORWARDING_PERIOD = 50_ms;
static constexpr uint32_t SUBSCRIBER_CACHE_SIZE = 128;
static constexpr uint32_t MAX_CHANNEL_NUMBER = iox::MAX_PORT_NUMBER;

} // namespace dds
} // namespace iox
