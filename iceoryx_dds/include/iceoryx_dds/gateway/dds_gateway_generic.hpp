#pragma once

#include <thread>
#include <atomic>

#include <iceoryx_posh/popo/gateway_generic.hpp>
#include <iceoryx_posh/capro/service_description.hpp>
#include <iceoryx_utils/cxx/vector.hpp>
#include <iceoryx_utils/internal/concurrent/smart_lock.hpp>

#include "iceoryx_dds/dds/dds_configs.hpp"
namespace iox {
namespace dds {

template <typename channel_t>
class DDSGatewayGeneric : public iox::popo::GatewayGeneric
{

    using ChannelFactory = std::function<channel_t(const iox::capro::ServiceDescription)>;
    using ChannelVector = iox::cxx::vector<channel_t, MAX_CHANNEL_NUMBER>;
    using ConcurrentChannelVector = iox::concurrent::smart_lock<ChannelVector>;

public:
    virtual ~DDSGatewayGeneric() noexcept;

    DDSGatewayGeneric(const DDSGatewayGeneric&) = delete;
    DDSGatewayGeneric& operator=(const DDSGatewayGeneric&) = delete;
    DDSGatewayGeneric(DDSGatewayGeneric&&) = delete;
    DDSGatewayGeneric& operator=(DDSGatewayGeneric&&) = delete;

    void runMultithreaded() noexcept;
    void shutdown() noexcept;

    virtual void discover(const iox::capro::CaproMessage& msg) noexcept = 0;
    virtual void forward() noexcept = 0;

    uint64_t getNumberOfChannels() const noexcept;

protected:
    DDSGatewayGeneric() noexcept;

    // These are made available to child classes for use in discover or forward methods.
    ChannelFactory m_channelFactory;
    ConcurrentChannelVector m_channels;

    void loadConfiguration() noexcept;
    channel_t setupChannel(const iox::capro::ServiceDescription& service) noexcept;
    void discardChannel(const iox::capro::ServiceDescription& service) noexcept;
    bool channelExists(const iox::capro::ServiceDescription& service) noexcept;

private:
    std::atomic_bool m_isRunning{false};
    std::atomic_bool m_runForwardingLoop{false};
    std::atomic_bool m_runDiscoveryLoop{false};

    std::thread m_discoveryThread;
    std::thread m_forwardingThread;

    void forwardingLoop() noexcept;
    void discoveryLoop() noexcept;

};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/gateway/dds_gateway_generic.inl"
