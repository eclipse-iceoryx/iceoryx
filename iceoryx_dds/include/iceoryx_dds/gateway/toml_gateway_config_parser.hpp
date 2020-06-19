#ifndef IOX_DDS_GATEWAY_FILE_CONFIG_PARSER_HPP
#define IOX_DDS_GATEWAY_FILE_CONFIG_PARSER_HPP

#include "iceoryx_dds/gateway/gateway_config.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/expected.hpp"

#include "cpptoml.h"


namespace iox {
namespace dds {

enum TomlGatewayConfigParseError
{
    FILE_NOT_FOUND,
    INVALID_SERVICE
};

static constexpr char defaultConfigFilePath[] = "/etc/iceoryx/gateway_config.toml";

class TomlGatewayConfigParser
{
public:
    static iox::cxx::expected<GatewayConfig, TomlGatewayConfigParseError> parse();
    static iox::cxx::expected<GatewayConfig, TomlGatewayConfigParseError> parse(ConfigFilePathString_t path);

protected:
    static iox::cxx::expected<TomlGatewayConfigParseError> validateConfig(const cpptoml::table& parsedToml) noexcept;
};

}
}

#endif // IOX_DDS_GATEWAY_FILE_CONFIG_PARSER_HPP
