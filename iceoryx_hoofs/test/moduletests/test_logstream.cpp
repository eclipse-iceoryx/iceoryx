// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_hoofs/cxx/convert.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/log/logstream.hpp"
#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_hoofs/testing/test.hpp"

#include <cstdint>
#include <limits>

namespace
{
using namespace ::testing;

class IoxLogStream_test : public Test
{
  public:
    Logger_Mock loggerMock;
};

TEST_F(IoxLogStream_test, CTor_Default)
{
    ::testing::Test::RecordProperty("TEST_ID", "209aadb5-9ea6-4620-a6d1-f8fb2d12b97d");
    iox::log::LogStream(loggerMock) << "";

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq(""));
    EXPECT_THAT(loggerMock.m_logs[0].level, Eq(iox::log::LogLevel::kWarn));
}

TEST_F(IoxLogStream_test, CTor_WithLogLevel)
{
    ::testing::Test::RecordProperty("TEST_ID", "af7a62db-8f22-4127-acee-83dec7ec6733");
    iox::log::LogStream(loggerMock, iox::log::LogLevel::kOff) << "";

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq(""));
    EXPECT_THAT(loggerMock.m_logs[0].level, Eq(iox::log::LogLevel::kOff));
}

TEST_F(IoxLogStream_test, UnnamedTemporaryLogStreamObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0f2c616-a373-4e6e-8083-c3bff5de58b4");
    const std::string claim = "The answer is ";
    const uint8_t answer = 42;
    const std::string bang = "!";

    iox::log::LogStream(loggerMock) << claim << answer << bang;

    std::string expected = claim + iox::cxx::convert::toString(answer) + bang;

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq(expected));
}

TEST_F(IoxLogStream_test, LocalLogStreamObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "0f4b4d78-e3c4-454a-a839-8f75b288e878");
    const std::string claim = "The answer is ";
    const uint8_t answer = 42;
    const std::string bang = "!";

    {
        auto log = iox::log::LogStream(loggerMock);
        log << claim;
        log << answer;
        log << bang;

        // the destructor flushes the log to the logger
        EXPECT_THAT(loggerMock.m_logs.size(), Eq(0u));
    }

    std::string expected = claim + iox::cxx::convert::toString(answer) + bang;

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq(expected));
}

TEST_F(IoxLogStream_test, ExplicitFlush)
{
    ::testing::Test::RecordProperty("TEST_ID", "42adbd23-7b14-4977-9f3a-779c0c9083fa");
    const std::string claim = "The answer is ";
    const uint8_t answer = 42;
    const std::string bang = "!";

    {
        auto log = iox::log::LogStream(loggerMock);
        log << claim;
        log.Flush();
        log << answer;
        log.Flush();
        log << bang;

        // the destructor flushes remaining logs to the logger
        EXPECT_THAT(loggerMock.m_logs.size(), Eq(2u));
    }

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(3u));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq(claim));
    EXPECT_THAT(loggerMock.m_logs[1].message, Eq(iox::cxx::convert::toString(answer)));
    EXPECT_THAT(loggerMock.m_logs[2].message, Eq(bang));
}

TEST_F(IoxLogStream_test, NoFlushWhenAlreadyFlushed)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb728d54-c741-4952-88a7-e8b2cebbd874");
    {
        auto log = iox::log::LogStream(loggerMock);
        log << "fubar";
        log.Flush();

        EXPECT_THAT(loggerMock.m_logs.size(), Eq(1u));
    }

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1u));
}

TEST_F(IoxLogStream_test, StreamOperatorCharArray)
{
    ::testing::Test::RecordProperty("TEST_ID", "68b034d7-a424-4e75-b6db-a5c4172ee271");
    char logValue[]{"This is the iceoryx logger!"};
    const char constLogValue[]{"Nothing to see here, move along!"};
    iox::log::LogStream(loggerMock) << logValue;
    iox::log::LogStream(loggerMock) << constLogValue;

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(2u));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq(std::string(logValue)));
    EXPECT_THAT(loggerMock.m_logs[1].message, Eq(std::string(constLogValue)));
}

TEST_F(IoxLogStream_test, StreamOperatorStdString)
{
    ::testing::Test::RecordProperty("TEST_ID", "da8dde06-3f69-4549-b584-64c3ad328dbc");
    std::string logValue{"This is the iceoryx logger!"};
    const std::string constLogValue{"Nothing to see here, move along!"};
    iox::log::LogStream(loggerMock) << logValue;
    iox::log::LogStream(loggerMock) << constLogValue;

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(2u));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq(logValue));
    EXPECT_THAT(loggerMock.m_logs[1].message, Eq(constLogValue));
}

TEST_F(IoxLogStream_test, StreamOperatorLogLevel)
{
    ::testing::Test::RecordProperty("TEST_ID", "d85b7ef4-35de-4e11-b0fd-f0de6581a9e6");
    std::string logValue{"This is the iceoryx logger!"};
    iox::log::LogLevel logLevel = iox::log::LogLevel::kWarn;
    iox::log::LogStream(loggerMock) << logValue << logLevel;

    EXPECT_THAT(loggerMock.m_logs[0].message, Eq("This is the iceoryx logger!Warn"));
}

TEST_F(IoxLogStream_test, StreamOperatorLogRawBuffer)
{
    ::testing::Test::RecordProperty("TEST_ID", "24974c62-3ec6-4a02-83ff-cbb61a3de664");
    struct DummyStruct
    {
        uint16_t a{0xAFFE};
        uint16_t b{0xDEAD};
        uint32_t c{0xC0FFEE};
    };

    DummyStruct d;

    iox::log::LogStream(loggerMock) << iox::log::RawBuffer(d);
    volatile uint16_t endianess{0x0100};
    auto bigEndian = reinterpret_cast<volatile uint8_t*>(&endianess);
    if (*bigEndian)
    {
        EXPECT_THAT(loggerMock.m_logs[0].message, Eq("0x[af fe de ad 00 c0 ff ee]"));
    }
    else
    {
        EXPECT_THAT(loggerMock.m_logs[0].message, Eq("0x[fe af ad de ee ff c0 00]"));
    }
}

template <class T>
class IoxLogStreamHexBin_test : public IoxLogStream_test
{
  public:
    T LogValueLow = std::numeric_limits<T>::lowest();
    T LogValueMin = std::numeric_limits<T>::min();
    T LogValueMax = std::numeric_limits<T>::max();
};

using LogHexBinTypes = Types<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t>;

TYPED_TEST_SUITE(IoxLogStreamHexBin_test, LogHexBinTypes);


template <typename LogType>
void testStreamOperatorLogHex(Logger_Mock& loggerMock, LogType logValue)
{
    iox::log::LogStream(loggerMock) << iox::log::HexFormat(logValue);

    // we need to check negative numbers in two's complement, therfore make the output value unsigned
    using TestType = typename std::make_unsigned<LogType>::type;
    auto outputValue = static_cast<TestType>(logValue);

    std::stringstream ss;
    ss << "0x" << std::hex << +outputValue; // the '+' is to prevent to interpret the (u)int8_t as char

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(loggerMock.m_logs[0].message, Eq(ss.str()));
}

TYPED_TEST(IoxLogStreamHexBin_test, StreamOperatorLogHex_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "bd47c99a-0808-4a19-bafb-580c65009e0d");
    testStreamOperatorLogHex(this->loggerMock, this->LogValueLow);
}

TYPED_TEST(IoxLogStreamHexBin_test, StreamOperatorLogHex_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "ee806b08-0e2a-49fd-b16b-5ad1c8da3150");
    testStreamOperatorLogHex(this->loggerMock, this->LogValueMin);
}

TYPED_TEST(IoxLogStreamHexBin_test, StreamOperatorLogHex_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "acfa2bbf-c2e1-42bf-88c6-2888d7d3a42a");
    testStreamOperatorLogHex(this->loggerMock, this->LogValueMax);
}

template <typename LogType>
void testStreamOperatorLogBin(Logger_Mock& loggerMock, LogType logValue)
{
    iox::log::LogStream(loggerMock) << iox::log::BinFormat(logValue);

    // we need to check negative numbers in two's complement, therfore make the output value unsigned
    using TestType = typename std::make_unsigned<LogType>::type;
    auto outputValue = static_cast<TestType>(logValue);

    ASSERT_THAT(loggerMock.m_logs.size(), Eq(1u));

    EXPECT_THAT(loggerMock.m_logs[0].message,
                Eq("0b" + std::bitset<std::numeric_limits<TestType>::digits>(outputValue).to_string()));
}

TYPED_TEST(IoxLogStreamHexBin_test, StreamOperatorLogBin_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4e684b7-5bcf-4e8d-8cb1-b6df95c3b37c");
    testStreamOperatorLogBin(this->loggerMock, this->LogValueLow);
}

TYPED_TEST(IoxLogStreamHexBin_test, StreamOperatorLogBin_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "f13b3e6a-8f7c-48c2-ae43-35e0a195556e");
    testStreamOperatorLogBin(this->loggerMock, this->LogValueMin);
}

TYPED_TEST(IoxLogStreamHexBin_test, StreamOperatorLogBin_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "b583014a-700f-46e3-8b7c-0a128c59598a");
    testStreamOperatorLogBin(this->loggerMock, this->LogValueMax);
}

using ArithmeticTypes =
    Types<bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, size_t, float, double>;

TYPED_TEST_SUITE(IoxLogStreamArithmetic_test, ArithmeticTypes);


template <typename Arithmetic>
class IoxLogStreamArithmetic_test : public IoxLogStream_test
{
  public:
    Arithmetic LogValueLow = std::numeric_limits<Arithmetic>::lowest();
    Arithmetic LogValueMin = std::numeric_limits<Arithmetic>::min();
    Arithmetic LogValueMax = std::numeric_limits<Arithmetic>::max();

    const Arithmetic ConstLogValueLow = std::numeric_limits<Arithmetic>::lowest();
    const Arithmetic ConstLogValueMin = std::numeric_limits<Arithmetic>::min();
    const Arithmetic ConstLogValueMax = std::numeric_limits<Arithmetic>::max();

    static constexpr Arithmetic ConstexprLogValueLow = std::numeric_limits<Arithmetic>::lowest();
    static constexpr Arithmetic ConstexprLogValueMin = std::numeric_limits<Arithmetic>::min();
    static constexpr Arithmetic ConstexprLogValueMax = std::numeric_limits<Arithmetic>::max();
};

template <typename Arithmetic>
constexpr Arithmetic IoxLogStreamArithmetic_test<Arithmetic>::ConstexprLogValueLow;

template <typename Arithmetic>
constexpr Arithmetic IoxLogStreamArithmetic_test<Arithmetic>::ConstexprLogValueMin;

template <typename Arithmetic>
constexpr Arithmetic IoxLogStreamArithmetic_test<Arithmetic>::ConstexprLogValueMax;

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "31f1504a-9353-4c46-9c8b-d7e430b07bd6");
    iox::log::LogStream(this->loggerMock) << this->LogValueLow;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->LogValueLow)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "e784ceb9-1e23-4e95-b667-855835897717");
    iox::log::LogStream(this->loggerMock) << this->LogValueMin;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->LogValueMin)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bff0182-07ad-4c7a-b8b7-3950a8aa9f4e");
    iox::log::LogStream(this->loggerMock) << this->LogValueMax;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->LogValueMax)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "65cfbc9b-a535-47fa-a543-0c31ba63d4ba");
    iox::log::LogStream(this->loggerMock) << this->ConstLogValueLow;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->ConstLogValueLow)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "fba70497-e252-4458-b00e-2dad8b94b8c8");
    iox::log::LogStream(this->loggerMock) << this->ConstLogValueMin;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->ConstLogValueMin)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5e28a6e-4321-4030-b53e-90089b3ee9b9");
    iox::log::LogStream(this->loggerMock) << this->ConstLogValueMax;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->ConstLogValueMax)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "e9688979-d209-4718-9810-49684fdd9261");
    iox::log::LogStream(this->loggerMock) << this->ConstexprLogValueLow;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->ConstexprLogValueLow)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "f6799599-582a-454c-85b8-b2059a5d50c6");
    iox::log::LogStream(this->loggerMock) << this->ConstexprLogValueMin;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->ConstexprLogValueMin)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a6dc777-a53b-4a42-9ab1-e1da893ad884");
    iox::log::LogStream(this->loggerMock) << this->ConstexprLogValueMax;

    ASSERT_THAT(this->loggerMock.m_logs.size(), Eq(1u));
    EXPECT_THAT(this->loggerMock.m_logs[0].message, Eq(iox::cxx::convert::toString(this->ConstexprLogValueMax)));
}
} // namespace
