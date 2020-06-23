
#include "iceoryx_dds/dds/cyclone_data_reader.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

// ======================================== Fixture ======================================== //
class CycloneDataReaderTest : public Test
{
  public:
    void SetUp(){};
    void TearDown(){};
};

// ======================================== Tests ======================================== //
TEST_F(CycloneDataReaderTest, DoesNotAttemptToReadWhenDisconnected)
{
    // ===== Setup
    iox::dds::CycloneDataReader reader{"", "", ""};
    uint64_t size = 1024;
    uint8_t buffer[size];

    // ===== Test
    auto result = reader.read(buffer, size);
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::dds::DataReaderError::NOT_CONNECTED, result.get_error());
}
