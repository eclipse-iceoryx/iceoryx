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

#include <array>
#include <cstdint>
#include <limits>

namespace
{
using namespace ::testing;

using iox::testing::Logger_Mock;

class LogStreamSut : public iox::log::LogStream
{
  public:
    explicit LogStreamSut(iox::log::Logger& logger)
        : iox::log::LogStream(logger, "file", 42, "function", iox::log::LogLevel::TRACE)
    {
    }
};

class IoxLogStream_test : public Test
{
  public:
    Logger_Mock loggerMock;
};

TEST_F(IoxLogStream_test, CTorDelegatesParameterToLogger)
{
    ::testing::Test::RecordProperty("TEST_ID", "209aadb5-9ea6-4620-a6d1-f8fb2d12b97d");
    constexpr const char* EXPECTED_FILE{"hypnotoad.hpp"};
    constexpr const char* EXPECTED_FUNCTION{"void all_glory_to_the_hypnotoad()"};
    constexpr int EXPECTED_LINE{42};
    constexpr auto EXPECTED_LOG_LEVEL{iox::log::LogLevel::WARN};
    iox::log::LogStream(loggerMock, EXPECTED_FILE, EXPECTED_LINE, EXPECTED_FUNCTION, EXPECTED_LOG_LEVEL) << "";

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs.back().file, StrEq(EXPECTED_FILE));
    EXPECT_THAT(loggerMock.logs.back().function, StrEq(EXPECTED_FUNCTION));
    EXPECT_THAT(loggerMock.logs.back().logLevel, Eq(EXPECTED_LOG_LEVEL));
    EXPECT_THAT(loggerMock.logs.back().message, Eq(""));
}

TEST_F(IoxLogStream_test, UnnamedTemporaryLogStreamObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "b0f2c616-a373-4e6e-8083-c3bff5de58b4");
    const std::string claim = "The answer is ";
    const uint8_t answer = 42;
    const std::string bang = "!";

    LogStreamSut(loggerMock) << claim << answer << bang;

    std::string expected = claim + iox::cxx::convert::toString(answer) + bang;

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(expected));
}

TEST_F(IoxLogStream_test, LocalLogStreamObject)
{
    ::testing::Test::RecordProperty("TEST_ID", "0f4b4d78-e3c4-454a-a839-8f75b288e878");
    const std::string claim = "The answer is ";
    const uint8_t answer = 42;
    const std::string bang = "!";

    {
        LogStreamSut sut(loggerMock);
        sut << claim;
        sut << answer;
        sut << bang;

        // the destructor flushes the log to the logger
        ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
        EXPECT_THAT(loggerMock.logs[0].message, StrEq(""));
    }

    std::string expected = claim + iox::cxx::convert::toString(answer) + bang;

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs.back().message, StrEq(expected));
}

TEST_F(IoxLogStream_test, StreamOperatorCStyleString)
{
    ::testing::Test::RecordProperty("TEST_ID", "68b034d7-a424-4e75-b6db-a5c4172ee271");
    std::string logValue("This is the iceoryx logger!");
    const std::string constLogValue{"Nothing to see here, move along!"};
    LogStreamSut(loggerMock) << logValue.c_str();
    LogStreamSut(loggerMock) << constLogValue.c_str();

    ASSERT_THAT(loggerMock.logs.size(), Eq(2U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(logValue));
    EXPECT_THAT(loggerMock.logs[1].message, StrEq(constLogValue));
}

TEST_F(IoxLogStream_test, StreamOperatorStdString)
{
    ::testing::Test::RecordProperty("TEST_ID", "da8dde06-3f69-4549-b584-64c3ad328dbc");
    std::string logValue{"This is the iceoryx logger!"};
    const std::string constLogValue{"Nothing to see here, move along!"};
    LogStreamSut(loggerMock) << logValue;
    LogStreamSut(loggerMock) << constLogValue;

    ASSERT_THAT(loggerMock.logs.size(), Eq(2U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(logValue));
    EXPECT_THAT(loggerMock.logs[1].message, StrEq(constLogValue));
}

TEST_F(IoxLogStream_test, StreamOperatorLogLevel)
{
    ::testing::Test::RecordProperty("TEST_ID", "d85b7ef4-35de-4e11-b0fd-f0de6581a9e6");
    std::string logValue{"This is the iceoryx logger!"};
    const auto logLevel = iox::log::LogLevel::WARN;
    LogStreamSut(loggerMock) << logValue << logLevel;

    EXPECT_THAT(loggerMock.logs[0].message, StrEq("This is the iceoryx logger!LogLevel::WARN"));
}

#if 0
/// @todo iox-#1755 re-enable when LogRawBuffer will be re-implemented

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

    LogStreamSut(loggerMock) << iox::log::RawBuffer(d);
    volatile uint16_t endianess{0x0100};
    auto bigEndian = reinterpret_cast<volatile uint8_t*>(&endianess);
    if (*bigEndian)
    {
        EXPECT_THAT(loggerMock.logs[0].message, Eq("0x[af fe de ad 00 c0 ff ee]"));
    }
    else
    {
        EXPECT_THAT(loggerMock.logs[0].message, Eq("0x[fe af ad de ee ff c0 00]"));
    }
}

#endif

template <class T>
class IoxLogStreamHexOctBinIntegral_test : public IoxLogStream_test
{
  public:
    T LogValueLow = std::numeric_limits<T>::lowest();
    T LogValueMin = std::numeric_limits<T>::min();
    T LogValueMax = std::numeric_limits<T>::max();
};

using LogHexOctBinIntegralTypes = Types<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t>;

TYPED_TEST_SUITE(IoxLogStreamHexOctBinIntegral_test, LogHexOctBinIntegralTypes, );

template <typename LogType>
void testStreamOperatorLogHex(Logger_Mock& loggerMock, LogType logValue)
{
    LogStreamSut(loggerMock) << iox::log::hex(logValue);

    // we need to check negative numbers in two's complement, therefore make the output value unsigned
    using TestType = typename std::make_unsigned<LogType>::type;
    auto outputValue = static_cast<TestType>(logValue);

    std::stringstream ss;
    ss << "0x" << std::hex << +outputValue; // the '+' is to prevent to interpret the (u)int8_t as char

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(ss.str()));
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogHex_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "bd47c99a-0808-4a19-bafb-580c65009e0d");
    testStreamOperatorLogHex(this->loggerMock, this->LogValueLow);
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogHex_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "ee806b08-0e2a-49fd-b16b-5ad1c8da3150");
    testStreamOperatorLogHex(this->loggerMock, this->LogValueMin);
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogHex_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "acfa2bbf-c2e1-42bf-88c6-2888d7d3a42a");
    testStreamOperatorLogHex(this->loggerMock, this->LogValueMax);
}

template <typename LogType>
void testStreamOperatorLogOct(Logger_Mock& loggerMock, LogType logValue)
{
    LogStreamSut(loggerMock) << iox::log::oct(logValue);

    // we need to check negative numbers in two's complement, therefore make the output value unsigned
    using TestType = typename std::make_unsigned<LogType>::type;
    auto outputValue = static_cast<TestType>(logValue);

    std::stringstream ss;
    ss << "0o" << std::oct << +outputValue; // the '+' is to prevent to interpret the (u)int8_t as char

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(ss.str()));
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogOct_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "0163ef2b-7e62-4ded-b093-d77cc4dc360e");
    testStreamOperatorLogOct(this->loggerMock, this->LogValueLow);
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogOct_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "68583225-9b47-486d-90da-31f6c1ca7480");
    testStreamOperatorLogOct(this->loggerMock, this->LogValueMin);
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogOct_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "c4d104c3-a12a-46e1-8755-cc4b9e0e480c");
    testStreamOperatorLogOct(this->loggerMock, this->LogValueMax);
}

#if 0
/// @todo iox-#1755 re-enable when LogBin will be re-implemented

template <typename LogType>
void testStreamOperatorLogBin(Logger_Mock& loggerMock, LogType logValue)
{
    iox::log::LogStream(loggerMock) << iox::log::BinFormat(logValue);

    // we need to check negative numbers in two's complement, therefore make the output value unsigned
    using TestType = typename std::make_unsigned<LogType>::type;
    auto outputValue = static_cast<TestType>(logValue);

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));

    EXPECT_THAT(loggerMock.logs[0].message,
                Eq("0b" + std::bitset<std::numeric_limits<TestType>::digits>(outputValue).to_string()));
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogBin_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4e684b7-5bcf-4e8d-8cb1-b6df95c3b37c");
    testStreamOperatorLogBin(this->loggerMock, this->LogValueLow);
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogBin_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "f13b3e6a-8f7c-48c2-ae43-35e0a195556e");
    testStreamOperatorLogBin(this->loggerMock, this->LogValueMin);
}

TYPED_TEST(IoxLogStreamHexOctBinIntegral_test, StreamOperatorLogBin_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "b583014a-700f-46e3-8b7c-0a128c59598a");
    testStreamOperatorLogBin(this->loggerMock, this->LogValueMax);
}

#endif

template <class T>
class IoxLogStreamHexFloatingPoint_test : public IoxLogStream_test
{
  public:
    T LogValueLow = std::numeric_limits<T>::lowest();
    T LogValueMin = std::numeric_limits<T>::min();
    T LogValueMax = std::numeric_limits<T>::max();
};

using LogHexFloatingPointTypes = Types<float, double, long double>;

TYPED_TEST_SUITE(IoxLogStreamHexFloatingPoint_test, LogHexFloatingPointTypes, );

template <typename T>
constexpr const char* floatingPointFormatSpecifier();
template <>
constexpr const char* floatingPointFormatSpecifier<float>()
{
    return "%a";
}
template <>
constexpr const char* floatingPointFormatSpecifier<double>()
{
    return "%la";
}
template <>
constexpr const char* floatingPointFormatSpecifier<long double>()
{
    return "%La";
}

template <typename LogType>
void testStreamOperatorLogHexFloatingPoint(Logger_Mock& loggerMock, LogType logValue)
{
    LogStreamSut(loggerMock) << iox::log::hex(logValue);

    constexpr uint64_t BUFFER_SIZE{1000};
    std::array<char, BUFFER_SIZE> buffer{0};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,hicpp-vararg) required to create a hex formatted float
    EXPECT_THAT(snprintf(buffer.data(), BUFFER_SIZE - 1, floatingPointFormatSpecifier<LogType>(), logValue), Gt(0));

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(buffer.data()));
}

TYPED_TEST(IoxLogStreamHexFloatingPoint_test, StreamOperatorLogHex_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "abcad269-7e1b-478a-9535-c9fff9ba1d05");
    testStreamOperatorLogHexFloatingPoint(this->loggerMock, this->LogValueLow);
}

TYPED_TEST(IoxLogStreamHexFloatingPoint_test, StreamOperatorLogHex_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "78d3a9c5-3f3e-410a-baba-942c5a1db46e");
    testStreamOperatorLogHexFloatingPoint(this->loggerMock, this->LogValueMin);
}

TYPED_TEST(IoxLogStreamHexFloatingPoint_test, StreamOperatorLogHex_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "e87982ae-58e7-4563-99bc-d08484fb60c9");
    testStreamOperatorLogHexFloatingPoint(this->loggerMock, this->LogValueMax);
}

using ArithmeticTypes =
    Types<bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, size_t, float, double>;

TYPED_TEST_SUITE(IoxLogStreamArithmetic_test, ArithmeticTypes, );

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

template <typename T>
std::string convertToString(const T val)
{
    return iox::cxx::convert::toString(val);
}

template <>
std::string convertToString<bool>(const bool val)
{
    return std::string(val ? "true" : "false");
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "31f1504a-9353-4c46-9c8b-d7e430b07bd6");
    LogStreamSut(this->loggerMock) << this->LogValueLow;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->LogValueLow)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "e784ceb9-1e23-4e95-b667-855835897717");
    LogStreamSut(this->loggerMock) << this->LogValueMin;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->LogValueMin)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bff0182-07ad-4c7a-b8b7-3950a8aa9f4e");
    LogStreamSut(this->loggerMock) << this->LogValueMax;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->LogValueMax)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "65cfbc9b-a535-47fa-a543-0c31ba63d4ba");
    LogStreamSut(this->loggerMock) << this->ConstLogValueLow;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstLogValueLow)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "fba70497-e252-4458-b00e-2dad8b94b8c8");
    LogStreamSut(this->loggerMock) << this->ConstLogValueMin;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstLogValueMin)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5e28a6e-4321-4030-b53e-90089b3ee9b9");
    LogStreamSut(this->loggerMock) << this->ConstLogValueMax;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstLogValueMax)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "e9688979-d209-4718-9810-49684fdd9261");
    LogStreamSut(this->loggerMock) << this->ConstexprLogValueLow;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstexprLogValueLow)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "f6799599-582a-454c-85b8-b2059a5d50c6");
    LogStreamSut(this->loggerMock) << this->ConstexprLogValueMin;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstexprLogValueMin)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a6dc777-a53b-4a42-9ab1-e1da893ad884");
    LogStreamSut(this->loggerMock) << this->ConstexprLogValueMax;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstexprLogValueMax)));
}

} // namespace
