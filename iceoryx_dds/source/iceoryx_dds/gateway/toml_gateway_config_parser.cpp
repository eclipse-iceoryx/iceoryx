
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
    iox::dds::LogInfo() << "Loading gateway config at: " << path;
}
