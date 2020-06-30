
#include "iceoryx_dds/gateway/toml_gateway_config_parser.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"

#include <regex>

iox::cxx::expected<iox::dds::GatewayConfig, iox::dds::TomlGatewayConfigParseError>
iox::dds::TomlGatewayConfigParser::parse()
{
    return iox::dds::TomlGatewayConfigParser::parse(DEFAULT_CONFIG_FILE_PATH);
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

    LogInfo() << "[TomlGatewayConfigParser] Using gateway config at: " << path;

    // Load the file
    auto parsedToml = cpptoml::parse_file(path.c_str());
    auto result = validate(*parsedToml);
    if (result.has_error())
    {
        LogWarn() << "[TomlGatewayConfigParser] Unable to parse configuration file";
        return iox::cxx::error<TomlGatewayConfigParseError>(result.get_error());
    }

    // Prepare config object
    auto serviceArray = parsedToml->get_table_array(GATEWAY_CONFIG_SERVICE_TABLE_NAME);
    for (const auto& service : *serviceArray)
    {
        GatewayConfig::ServiceEntry entry;
        auto serviceName = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_NAME);
        auto instance = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_INSTANCE_NAME);
        auto event = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_EVENT_NAME);
        entry.m_serviceDescription =
            iox::capro::ServiceDescription(iox::capro::IdString(iox::cxx::TruncateToCapacity, *serviceName),
                                           iox::capro::IdString(iox::cxx::TruncateToCapacity, *instance),
                                           iox::capro::IdString(iox::cxx::TruncateToCapacity, *event));
        auto size = service->get_as<uint64_t>(GATEWAY_CONFIG_SERVICE_PAYLOAD_SIZE);
        entry.m_dataSize = *size;
        config.m_configuredServices.push_back(entry);
        LogDebug() << "[TomlGatewayConfigParser] Found service: {" << *serviceName << ", " << *instance << ", "
                   << *event << "}";
    }

    return iox::cxx::success<GatewayConfig>(config);
}

iox::cxx::expected<iox::dds::TomlGatewayConfigParseError>
iox::dds::TomlGatewayConfigParser::validate(const cpptoml::table& parsedToml) noexcept
{
    // Check for expected fields
    auto serviceArray = parsedToml.get_table_array(GATEWAY_CONFIG_SERVICE_TABLE_NAME);
    if (!serviceArray)
    {
        LogError() << "[TomlGatewayConfigParser] Incomplete configuration provided.";
        return iox::cxx::error<TomlGatewayConfigParseError>(TomlGatewayConfigParseError::INCOMPLETE_CONFIGURATION);
    }

    uint8_t count = 0;
    for (const auto& service : *serviceArray)
    {
        ++count;

        auto serviceName = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_NAME);
        auto instance = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_INSTANCE_NAME);
        auto event = service->get_as<std::string>(GATEWAY_CONFIG_SERVICE_EVENT_NAME);
        auto sampleSize = service->get_as<uint64_t>(GATEWAY_CONFIG_SERVICE_PAYLOAD_SIZE);

        // check for incomplete service descriptions
        if (!serviceName || !instance || !event)
        {
            LogError() << "[TomlGatewayConfigParser] Incomplete service description at entry: " << count;
            return iox::cxx::error<TomlGatewayConfigParseError>(
                TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION);
        }
        // check for incomplete service descriptions
        if (!sampleSize)
        {
            LogError() << "[TomlGatewayConfigParser] Incomplete data description at entry: " << count;
            return iox::cxx::error<TomlGatewayConfigParseError>(
                TomlGatewayConfigParseError::INCOMPLETE_SERVICE_DESCRIPTION);
        }

        // check for invalid characters in strings
        if (hasInvalidCharacter(*serviceName) || hasInvalidCharacter(*instance) || hasInvalidCharacter(*event))
        {
            LogError() << "[TomlGatewayConfigParser] Invalid service description at entry: " << count;
            return iox::cxx::error<TomlGatewayConfigParseError>(
                TomlGatewayConfigParseError::INVALID_SERVICE_DESCRIPTION);
        }
    }

    return iox::cxx::success<>();
}

bool iox::dds::TomlGatewayConfigParser::hasInvalidCharacter(std::string s) noexcept
{
    // See: https://design.ros2.org/articles/topic_and_service_names.html
    const std::regex regex(REGEX_VALID_CHARACTERS);
    auto isInvalid = !std::regex_match(s, regex);
    if (isInvalid)
    {
        LogError() << "[TomlGatewayConfigParser] Invalid character in name: " + s;
    }
    return isInvalid;
}
