// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_hoofs/testing/test.hpp"
#include "iox/log/logstream.hpp"
#include "iox/logging.hpp"

#include <array>
#include <bitset>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>

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

    std::string expected = claim + std::to_string(answer) + bang;

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

    std::string expected = claim + std::to_string(answer) + bang;

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

TEST_F(IoxLogStream_test, StreamOperatorChar)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a1fff17-e388-4f84-bb16-30bb3432ae9d");
    char logValue{'b'};
    const char constLogValue{'o'};
    constexpr char CONSTEXPR_LOG_VALUE{'b'};
    LogStreamSut(loggerMock) << logValue;
    LogStreamSut(loggerMock) << constLogValue;
    LogStreamSut(loggerMock) << CONSTEXPR_LOG_VALUE;

    ASSERT_THAT(loggerMock.logs.size(), Eq(3U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq("b"));
    EXPECT_THAT(loggerMock.logs[1].message, StrEq("o"));
    EXPECT_THAT(loggerMock.logs[2].message, StrEq("b"));
}

TEST_F(IoxLogStream_test, StreamOperator8BitTypesWithCharAsCharacterAndEverythingElseAsNumber)
{
    ::testing::Test::RecordProperty("TEST_ID", "707d4c04-1999-4713-b930-e113969617e0");
    char cc{'a'};
    signed char sc{'a'};
    unsigned char uc{'a'};
    int8_t i8{'a'};
    uint8_t u8{'a'};

    LogStreamSut(loggerMock) << cc;
    LogStreamSut(loggerMock) << sc;
    LogStreamSut(loggerMock) << uc;
    LogStreamSut(loggerMock) << i8;
    LogStreamSut(loggerMock) << u8;

    ASSERT_THAT(loggerMock.logs.size(), Eq(5U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq("a"));
    EXPECT_THAT(loggerMock.logs[1].message, StrEq("97"));
    EXPECT_THAT(loggerMock.logs[2].message, StrEq("97"));
    EXPECT_THAT(loggerMock.logs[3].message, StrEq("97"));
    EXPECT_THAT(loggerMock.logs[4].message, StrEq("97"));
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

template <typename LogType>
void testStreamOperatorLogBin(Logger_Mock& loggerMock, LogType logValue)
{
    LogStreamSut(loggerMock) << iox::log::bin(logValue);

    // we need to check negative numbers in two's complement, therefore make the output value unsigned
    using TestType = typename std::make_unsigned<LogType>::type;
    auto outputValue = static_cast<TestType>(logValue);

    auto expectedValue = "0b" + std::bitset<std::numeric_limits<TestType>::digits>(outputValue).to_string();

    ASSERT_THAT(loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(loggerMock.logs[0].message, StrEq(expectedValue));
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

template <typename T>
class Wrapper
{
  public:
    constexpr explicit Wrapper(T value) noexcept
        : m_value(value)
    {
    }

    // NOLINTJUSTIFICATION Implicit conversion is required for the test
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    constexpr operator T() const noexcept
    {
        return m_value;
    }

  private:
    T m_value{};
};

using ArithmeticTypes =
    Types<bool, int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t, uint64_t, size_t, float, double>;

TYPED_TEST_SUITE(IoxLogStreamArithmetic_test, ArithmeticTypes, );

template <typename Arithmetic>
class IoxLogStreamArithmetic_test : public IoxLogStream_test
{
  public:
    using type_t = Arithmetic;

    Arithmetic LogValueLow = std::numeric_limits<Arithmetic>::lowest();
    Arithmetic LogValueMin = std::numeric_limits<Arithmetic>::min();
    Arithmetic LogValueMax = std::numeric_limits<Arithmetic>::max();

    const Arithmetic ConstLogValueLow = std::numeric_limits<Arithmetic>::lowest();
    const Arithmetic ConstLogValueMin = std::numeric_limits<Arithmetic>::min();
    const Arithmetic ConstLogValueMax = std::numeric_limits<Arithmetic>::max();

    static constexpr Arithmetic CONSTEXPR_LOG_VALUE_LOW = std::numeric_limits<Arithmetic>::lowest();
    static constexpr Arithmetic CONSTEXPR_LOG_VALUE_MIN = std::numeric_limits<Arithmetic>::min();
    static constexpr Arithmetic CONSTEXPR_LOG_VALUE_MAX = std::numeric_limits<Arithmetic>::max();
};

template <typename Arithmetic>
constexpr Arithmetic IoxLogStreamArithmetic_test<Arithmetic>::CONSTEXPR_LOG_VALUE_LOW;

template <typename Arithmetic>
constexpr Arithmetic IoxLogStreamArithmetic_test<Arithmetic>::CONSTEXPR_LOG_VALUE_MIN;

template <typename Arithmetic>
constexpr Arithmetic IoxLogStreamArithmetic_test<Arithmetic>::CONSTEXPR_LOG_VALUE_MAX;

template <typename T>
std::string convertToString(const T val)
{
    return std::to_string(val);
}

template <>
std::string convertToString<float>(const float val)
{
    std::stringstream ss;
    ss << val;
    return ss.str();
}

template <>
std::string convertToString<double>(const double val)
{
    std::stringstream ss;
    ss << val;
    return ss.str();
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

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "86dccc28-0007-4ab3-88c6-216eeb861fef");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->LogValueLow);

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->LogValueLow)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "72bc51fe-a23f-4c7d-a17f-0e4ad29bee9f");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->LogValueMin);

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->LogValueMin)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "49e79c8b-98c0-4074-aed1-25293bf49bd0");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->LogValueMax);

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

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ConstValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "77cb68ac-bffb-4cf5-a4c2-119e5aaae307");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->ConstLogValueLow);

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstLogValueLow)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ConstValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "3b042ec2-0a6d-4d24-9a12-dab81736847f");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->ConstLogValueMin);

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstLogValueMin)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ConstValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "83194cc1-3c2a-4c0e-a413-972a95f36b66");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->ConstLogValueMax);

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->ConstLogValueMax)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "e9688979-d209-4718-9810-49684fdd9261");
    LogStreamSut(this->loggerMock) << this->CONSTEXPR_LOG_VALUE_LOW;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->CONSTEXPR_LOG_VALUE_LOW)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "f6799599-582a-454c-85b8-b2059a5d50c6");
    LogStreamSut(this->loggerMock) << this->CONSTEXPR_LOG_VALUE_MIN;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->CONSTEXPR_LOG_VALUE_MIN)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_ConstexprValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a6dc777-a53b-4a42-9ab1-e1da893ad884");
    LogStreamSut(this->loggerMock) << this->CONSTEXPR_LOG_VALUE_MAX;

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->CONSTEXPR_LOG_VALUE_MAX)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ConstexprValueLow)
{
    ::testing::Test::RecordProperty("TEST_ID", "3025b6e3-3abd-49e8-8d3c-f6fa0c164011");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->CONSTEXPR_LOG_VALUE_LOW);

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->CONSTEXPR_LOG_VALUE_LOW)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ConstexprValueMin)
{
    ::testing::Test::RecordProperty("TEST_ID", "c49ad58d-948a-499a-9e2b-8bbdeb9b5c5e");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->CONSTEXPR_LOG_VALUE_MIN);

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->CONSTEXPR_LOG_VALUE_MIN)));
}

TYPED_TEST(IoxLogStreamArithmetic_test, StreamOperator_WrappedAndImplicitlyConverted_ConstexprValueMax)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c7637ce-8155-4edc-88e2-a155169934f2");
    using type_t = typename TestFixture::type_t;
    LogStreamSut(this->loggerMock) << Wrapper<type_t>(this->CONSTEXPR_LOG_VALUE_MAX);

    ASSERT_THAT(this->loggerMock.logs.size(), Eq(1U));
    EXPECT_THAT(this->loggerMock.logs[0].message, StrEq(convertToString(this->CONSTEXPR_LOG_VALUE_MAX)));
}

} // namespace
