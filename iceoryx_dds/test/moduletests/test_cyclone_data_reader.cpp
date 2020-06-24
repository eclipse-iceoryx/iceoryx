
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
    uint64_t bufferSize = 1024;
    uint8_t buffer[bufferSize];
    uint64_t sampleSize = sizeof(uint64_t);

    // ===== Test
    iox::dds::CycloneDataReader reader{"", "", ""};
    auto result = reader.read(buffer, bufferSize, sampleSize);
    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::dds::DataReaderError::NOT_CONNECTED, result.get_error());
}

TEST_F(CycloneDataReaderTest, ReturnsErrorWhenAttemptingToReadIntoANullBuffer)
{
    // ===== Setup
    uint64_t bufferSize = 0;
    uint8_t* buffer = nullptr;
    uint64_t sampleSize = sizeof(uint64_t);

    // ===== Test
    iox::dds::CycloneDataReader reader{"", "", ""};
    reader.connect();
    auto result = reader.read(buffer, bufferSize, sampleSize);

    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::dds::DataReaderError::INVALID_RECV_BUFFER, result.get_error());
}

TEST_F(CycloneDataReaderTest, ReturnsErrorWhenReceiverBufferSmallerThanSampleSize)
{
    // ===== Setup
    uint64_t bufferSize = 0;
    uint8_t buffer[bufferSize];
    uint64_t sampleSize = sizeof(uint64_t);

    // ===== Test
    iox::dds::CycloneDataReader reader{"", "", ""};
    reader.connect();
    auto result = reader.read(buffer, bufferSize, sampleSize);

    EXPECT_EQ(true, result.has_error());
    EXPECT_EQ(iox::dds::DataReaderError::INVALID_RECV_BUFFER, result.get_error());
}
