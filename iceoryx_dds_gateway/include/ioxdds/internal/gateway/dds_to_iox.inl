
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

    m_channelFactory = InputChannel<publisher_t, data_reader_t>::create;

    // Initialize pre-configured services
    loadConfiguration();

}

template <typename publisher_t, typename data_reader_t>
inline DDS2IceoryxGateway<publisher_t, data_reader_t>::~DDS2IceoryxGateway()
{
}

template <typename publisher_t, typename data_reader_t>
inline void DDS2IceoryxGateway<publisher_t, data_reader_t>::runMultithreaded() noexcept
{
}

template <typename publisher_t, typename data_reader_t>
inline void DDS2IceoryxGateway<publisher_t, data_reader_t>::discoveryLoop() noexcept
{

}

template <typename publisher_t, typename data_reader_t>
inline void DDS2IceoryxGateway<publisher_t, data_reader_t>::forwardingLoop() noexcept
{

}

template <typename publisher_t, typename data_reader_t>
inline void DDS2IceoryxGateway<publisher_t, data_reader_t>::shutdown() noexcept
{

}

// ======================================== Private ======================================== //
template <typename publisher_t, typename data_reader_t>
inline void DDS2IceoryxGateway<publisher_t, data_reader_t>::loadConfiguration() noexcept
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

        setupChannel(iox::capro::ServiceDescription(
                         IdString(iox::cxx::TruncateToCapacity, service.c_str()),
                         IdString(iox::cxx::TruncateToCapacity, instance.c_str()),
                         IdString(iox::cxx::TruncateToCapacity, event.c_str())
                         )
                     );

        LogDebug() << "Loaded topic from file: " + service + "/" + instance + "/" + event;

    }

}

template <typename publisher_t, typename data_reader_t>
InputChannel<publisher_t, data_reader_t> DDS2IceoryxGateway<publisher_t, data_reader_t>::setupChannel(
    const iox::capro::ServiceDescription& service) noexcept
{
    auto channel = m_channelFactory(service);
    iox::LogDebug() << "[DDS2IceoryxGateway] Input channel set up for service: "
                    << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                    << service.getEventIDString();
    m_channels->push_back(channel);
    return channel;
}

template <typename publisher_t, typename data_reader_t>
void DDS2IceoryxGateway<publisher_t, data_reader_t>::discardChannel(
    const iox::capro::ServiceDescription& service) noexcept
{
    auto guardedVector = m_channels.GetScopeGuard();
    auto channel = std::find_if(
        guardedVector->begin(), guardedVector->end(), [&service](const InputChannel<publisher_t, data_reader_t>& channel) {
            return channel.getService() == service;
        });
    if (channel != guardedVector->end())
    {
        guardedVector->erase(channel);
        iox::LogDebug() << "[DDS2IceoryxGateway] Input channel taken down for service: "
                        << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                        << service.getEventIDString();
    }
}


} // namespace dds
} // namespace iox
