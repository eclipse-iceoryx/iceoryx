
#include "iceoryx_dds/gateway/toml_gateway_config_parser.hpp"
#include "stubs/stubbed_toml_gateway_config_parser.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::_;

// ======================================== Helpers ======================================== //


// ======================================== Fixture ======================================== //
class TomlGatewayConfigParserTest : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

// ======================================== Tests ======================================== //
TEST_F(TomlGatewayConfigParserTest, IgnoresServicesWithHyphensInTheirName)
{

    // ===== Setup

    // Prepare configuration to test with
    auto toml = cpptoml::make_table();
    auto serviceArray = cpptoml::make_table_array();
    auto serviceEntry = cpptoml::make_table();
    serviceEntry->insert("service", "service-with-hyphens");
    serviceEntry->insert("instance", "instance-with-hyphens");
    serviceEntry->insert("event", "event-with-hyphens");
    serviceArray->push_back(serviceEntry);
    toml->insert("services", serviceArray);

    // ===== Test
    auto result = iox::dds::StubbedTomlGatewayConfigParser::validateConfig(*toml);
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::dds::TomlGatewayConfigParseError::INVALID_SERVICE, result.get_error());

}
