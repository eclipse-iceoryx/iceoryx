#ifndef IOX_DDS_TEST_STUBS_STUBBED_TOML_GATEWAY_CONFIG_PARSER_HPP
#define IOX_DDS_TEST_STUBS_STUBBED_TOML_GATEWAY_CONFIG_PARSER_HPP

#include "iceoryx_dds/gateway/toml_gateway_config_parser.hpp"

#include "iceoryx_utils/cxx/expected.hpp"

namespace iox
{
namespace dds
{
///
/// @brief The StubbedTomlGatewayConfigParser class exposes protected methods in the TomlGatewayConfigParser os that
/// they can be tested.
///
class StubbedTomlGatewayConfigParser : public iox::dds::TomlGatewayConfigParser
{
  public:
    static iox::cxx::expected<TomlGatewayConfigParseError> validate(const cpptoml::table& parsedToml) noexcept
    {
        return iox::dds::TomlGatewayConfigParser::validate(parsedToml);
    }
};

} // namespace dds
} // namespace iox

#endif // IOX_DDS_TEST_STUBS_STUBBED_TOML_GATEWAY_CONFIG_PARSER_HPP
