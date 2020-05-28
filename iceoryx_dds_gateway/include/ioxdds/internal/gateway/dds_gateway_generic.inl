
#include <cpptoml.h>

#include "ioxdds/internal/log/logging.hpp"

#include "ioxdds/dds/dds_types.hpp"
#include "ioxdds/gateway/dds_gateway_generic.hpp"

template<typename channel_t>
iox::dds::DDSGatewayGeneric<channel_t>::DDSGatewayGeneric() : iox::popo::GatewayGeneric(iox::capro::Interfaces::DDS)
{
    m_channelFactory = channel_t::create;
}

template<typename channel_t>
void iox::dds::DDSGatewayGeneric<channel_t>::loadConfiguration() noexcept
{
    // Search for config passed as command line argument.


    // Search for local config.
    auto config = cpptoml::parse_file("config.toml");

    // Search for local config.


    // Setup data readers and publishers
    auto configuredTopics = config->get_table_array("services");
    for(const auto& topic : *configuredTopics)
    {
        auto service = topic->get_as<std::string>("service").value_or("");
        auto instance = topic->get_as<std::string>("instance").value_or("");
        auto event = topic->get_as<std::string>("event").value_or("");

        this->setupChannel(iox::capro::ServiceDescription(
                         IdString(iox::cxx::TruncateToCapacity, service.c_str()),
                         IdString(iox::cxx::TruncateToCapacity, instance.c_str()),
                         IdString(iox::cxx::TruncateToCapacity, event.c_str())
                         )
                     );

        LogDebug() << "[DDSGatewayGeneric] Loaded topic from file: " + service + "/" + instance + "/" + event;

    }
}


template<typename channel_t>
channel_t iox::dds::DDSGatewayGeneric<channel_t>::setupChannel(const iox::capro::ServiceDescription& service) noexcept
{
    auto channel = m_channelFactory(service);
    iox::LogDebug() << "[DDSGatewayGeneric] Channel set up for service: "
                    << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                    << service.getEventIDString();
    m_channels->push_back(channel);
    return channel;
}

template<typename channel_t>
void iox::dds::DDSGatewayGeneric<channel_t>::discardChannel(const iox::capro::ServiceDescription& service) noexcept
{
    auto guardedVector = m_channels.GetScopeGuard();
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
