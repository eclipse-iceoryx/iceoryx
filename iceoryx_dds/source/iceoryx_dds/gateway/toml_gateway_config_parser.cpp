
#include "iceoryx_dds/gateway/toml_gateway_config_parser.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"

iox::cxx::expected<iox::dds::GatewayConfig, iox::dds::TomlGatewayConfigParseError>
iox::dds::TomlGatewayConfigParser::parse()
{
    return iox::dds::TomlGatewayConfigParser::parse(iox::dds::defaultConfigFilePath);
}


iox::cxx::expected<iox::dds::GatewayConfig, iox::dds::TomlGatewayConfigParseError>
iox::dds::TomlGatewayConfigParser::parse(ConfigFilePathString_t path)
{
    GatewayConfig config;

    // Set defaults if no path provided.
    if (path.size() == 0)
    {
        config.setDefaults();
        return iox::cxx::success<iox::dds::GatewayConfig>(config);
    }

    LogInfo() << "[TomlGatewayConfigParser] Loading gateway config at: " << path;

    auto parsedToml = cpptoml::parse_file(path.c_str());
    auto serviceArray = parsedToml->get_table_array("services");

    uint8_t count = 0;
    for(const auto& service : *serviceArray)
    {
        ++count;
        auto name = service->get_as<std::string>("service");
        auto instance = service->get_as<std::string>("instance");
        auto event = service->get_as<std::string>("event");

        // Ignore incomplete service descriptions
        if(!name || !instance || !event)
        {
            LogWarn() << "[TomlGatewayConfigParser] Incomplete service description at entry: " << count;
            continue;
        }

        config.m_configuredServices.push_back(iox::capro::ServiceDescription(
                                                  iox::capro::IdString(iox::cxx::TruncateToCapacity, *name),
                                                  iox::capro::IdString(iox::cxx::TruncateToCapacity, *instance),
                                                  iox::capro::IdString(iox::cxx::TruncateToCapacity, *event)));

        LogDebug() << "[TomlGatewayConfigParser] Loaded service: {" << *name << ", " << *instance << ", " << *event << "}";
    }

    return iox::cxx::success<GatewayConfig>(config);

}
