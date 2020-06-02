
#include <cpptoml.h>

#include "ioxdds/internal/log/logging.hpp"

#include "ioxdds/dds/dds_types.hpp"
#include "ioxdds/gateway/dds_gateway_generic.hpp"

// ================================================== Public ================================================== //

template<typename channel_t>
inline iox::dds::DDSGatewayGeneric<channel_t>::~DDSGatewayGeneric() noexcept
{
    shutdown();
}

template<typename channel_t>
inline void iox::dds::DDSGatewayGeneric<channel_t>::runMultithreaded() noexcept
{
    m_discoveryThread = std::thread([this] { discoveryLoop(); });
    m_forwardingThread = std::thread([this] { forwardingLoop(); });
    m_isRunning.store(true, std::memory_order_relaxed);
}

template<typename channel_t>
inline void iox::dds::DDSGatewayGeneric<channel_t>::shutdown() noexcept
{
    if (m_isRunning.load(std::memory_order_relaxed))
    {
        iox::LogDebug() << "[DDSGatewayGeneric] Shutting down Posh2DDSGateway.";

        m_runDiscoveryLoop.store(false, std::memory_order_relaxed);
        m_runForwardingLoop.store(false, std::memory_order_relaxed);

        m_discoveryThread.join();
        m_forwardingThread.join();

        m_isRunning.store(false, std::memory_order_relaxed);
    }
}

template <typename channel_t>
inline uint64_t iox::dds::DDSGatewayGeneric<channel_t>::getNumberOfChannels() const noexcept
{
    auto guardedVector = m_channels.GetScopeGuard();
    return guardedVector->size();
}

// ================================================== Protected ================================================== //

template<typename channel_t>
inline iox::dds::DDSGatewayGeneric<channel_t>::DDSGatewayGeneric() noexcept : iox::popo::GatewayGeneric(iox::capro::Interfaces::DDS)
{
     LogDebug() << "[DDSGatewayGeneric] Using default channel factory.";
    m_channelFactory = channel_t::create;
}

template<typename channel_t>
inline void iox::dds::DDSGatewayGeneric<channel_t>::loadConfiguration() noexcept
{
    // Search for config passed as command line argument.


    // Search for local config.
    auto config = cpptoml::parse_file("config.toml");

    // Search for local config.


    // Setup data readers and publishers                  
    auto configuredTopics = config->get_table_array("services");  
    LogDebug() << "[DDSGatewayGeneric] Setting up channels for pre-configured services.";
    for(const auto& topic : *configuredTopics)
    {
        auto service = topic->get_as<std::string>("service").value_or("");
        auto instance = topic->get_as<std::string>("instance").value_or("");
        auto event = topic->get_as<std::string>("event").value_or("");

        this->setupChannel(iox::capro::ServiceDescription(
                             IdString(iox::cxx::TruncateToCapacity, service.c_str()),
                             IdString(iox::cxx::TruncateToCapacity, instance.c_str()),
                             IdString(iox::cxx::TruncateToCapacity, event.c_str())
                         ));
    }
}


template<typename channel_t>
inline channel_t iox::dds::DDSGatewayGeneric<channel_t>::setupChannel(const iox::capro::ServiceDescription& service) noexcept
{
    auto channel = m_channelFactory(service);
    m_channels->push_back(channel);
    iox::LogDebug() << "[DDSGatewayGeneric] Channel set up for service: "
                    << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                    << service.getEventIDString();
    return channel;
}

template<typename channel_t>
inline void iox::dds::DDSGatewayGeneric<channel_t>::discardChannel(const iox::capro::ServiceDescription& service) noexcept
{
    auto guardedVector = this->m_channels.GetScopeGuard();
    auto channel = std::find_if(
        guardedVector->begin(), guardedVector->end(), [&service](const channel_t& channel) {
            return channel.getService() == service;
        });
    if (channel != guardedVector->end())
    {
        guardedVector->erase(channel);
        iox::LogDebug() << "[DDSGatewayGeneric] Channel taken down for service: "
                        << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                        << service.getEventIDString();
    }
}

template<typename channel_t>
inline bool iox::dds::DDSGatewayGeneric<channel_t>::channelExists(const iox::capro::ServiceDescription& service) noexcept
{
    auto guardedVector = this->m_channels.GetScopeGuard();
    auto channel = std::find_if(
        guardedVector->begin(), guardedVector->end(), [&service](const channel_t& channel) {
            return channel.getService() == service;
        });
    return channel != guardedVector->end();
}

// ================================================== Private ================================================== //

template<typename channel_t>
inline void iox::dds::DDSGatewayGeneric<channel_t>::discoveryLoop() noexcept
{
    iox::LogDebug() << "[DDSGatewayGeneric] Starting discovery.";
    m_runDiscoveryLoop.store(true, std::memory_order_relaxed);
    while (m_runDiscoveryLoop.load(std::memory_order_relaxed))
    {
        iox::capro::CaproMessage msg;
        while (this->getCaProMessage(msg))
        {
            discover(msg);
        }
        std::this_thread::sleep_until(std::chrono::steady_clock::now()
                                      + std::chrono::milliseconds(DISCOVERY_PERIOD.milliSeconds<int64_t>()));
    }
    iox::LogDebug() << "[DDSGatewayGeneric] Stopped discovery.";
}

template<typename channel_t>
inline void iox::dds::DDSGatewayGeneric<channel_t>::forwardingLoop() noexcept
{
    iox::LogDebug() << "[DDSGatewayGeneric] Starting forwarding.";
    m_runForwardingLoop.store(true, std::memory_order_relaxed);
    while (m_runForwardingLoop.load(std::memory_order_relaxed))
    {
        forward();
        std::this_thread::sleep_until(std::chrono::steady_clock::now()
                                      + std::chrono::milliseconds(FORWARDING_PERIOD.milliSeconds<int64_t>()));
    };
    iox::LogDebug() << "[DDSGatewayGeneric] Stopped forwarding.";
}

