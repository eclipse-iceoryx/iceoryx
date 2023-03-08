// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"
#include "test.hpp"
#include <gtest/gtest.h>

#include "iceoryx_hoofs/error_reporting/location.hpp"
#include "iceoryx_hoofs/testing/error_reporting/test_error_handler.hpp"

// NOLINTNEXTLINE(hicpp-deprecated-headers) required to work on some platforms
#include <setjmp.h>
#include <thread>

namespace
{
using namespace ::testing;
using namespace iox::err;
using namespace iox::testing;
using iox::err::ErrorDescriptor;

constexpr ErrorCode CODE1{73};
constexpr ErrorCode CODE2{37};
constexpr ErrorCode CODE3{21};
constexpr ErrorCode VIOLATION{12};

constexpr ModuleId MODULE{66};

class TestHandler_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    TestHandler sut;

    bool hasPanicked() const
    {
        return sut.hasPanicked();
    }

    bool hasError() const
    {
        return sut.hasError();
    }

    bool hasError(ErrorCode code) const
    {
        return sut.hasError(code);
    }

    bool hasViolation() const
    {
        return sut.hasViolation(ErrorCode(VIOLATION));
    }

    bool hasAnyError() const
    {
        return hasPanicked() || hasError() || hasViolation();
    }
};

TEST_F(TestHandler_test, constructionAndDestructionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "09f24453-aea1-4128-83f3-929337b9892a");
    EXPECT_FALSE(hasAnyError());
}

TEST_F(TestHandler_test, panicWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e2c5e639-722f-4bab-85c7-98268345b033");
    sut.panic();
    EXPECT_TRUE(sut.hasPanicked());
    EXPECT_FALSE(sut.hasError());

    sut.reset();
    EXPECT_FALSE(hasAnyError());
}

TEST_F(TestHandler_test, reportErrorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "90bd13cf-ece2-4221-8cce-7b2a99568a6a");
    sut.reportError(ErrorDescriptor{CURRENT_SOURCE_LOCATION, CODE1, MODULE});
    EXPECT_FALSE(sut.hasPanicked());
    EXPECT_TRUE(sut.hasError());
    EXPECT_TRUE(sut.hasError(CODE1, MODULE));

    sut.reset();
    EXPECT_FALSE(hasAnyError());
    EXPECT_FALSE(hasError(CODE1)); // checked for consistency
}

TEST_F(TestHandler_test, reportViolationWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5746886e-7309-4435-9e0a-2e6856a318f5");
    sut.reportViolation(ErrorDescriptor{CURRENT_SOURCE_LOCATION, VIOLATION, MODULE});

    EXPECT_TRUE(hasViolation());

    sut.reset();
    EXPECT_FALSE(hasAnyError());
}

TEST_F(TestHandler_test, hasErrorDetectsOnlyreportErroredErrors)
{
    ::testing::Test::RecordProperty("TEST_ID", "0ee52915-88b7-4041-9f63-93ec5c882e95");
    sut.reportError(ErrorDescriptor{CURRENT_SOURCE_LOCATION, CODE1, MODULE});
    sut.reportError(ErrorDescriptor{CURRENT_SOURCE_LOCATION, CODE2, MODULE});

    EXPECT_FALSE(sut.hasPanicked());
    EXPECT_TRUE(sut.hasError(CODE1, MODULE));
    EXPECT_TRUE(sut.hasError(CODE2, MODULE));
    EXPECT_FALSE(sut.hasError(CODE3, MODULE));

    sut.reset();
    EXPECT_FALSE(sut.hasError(CODE1, MODULE));
    EXPECT_FALSE(sut.hasError(CODE2, MODULE));
    EXPECT_FALSE(sut.hasError(CODE3, MODULE));
}

TEST_F(TestHandler_test, resettingMultipleErrorsWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "9715c394-5576-4fd8-a0f6-24560f60c161");
    sut.reportError(ErrorDescriptor{CURRENT_SOURCE_LOCATION, CODE1, MODULE});
    sut.reportError(ErrorDescriptor{CURRENT_SOURCE_LOCATION, CODE2, MODULE});
    sut.reportViolation(ErrorDescriptor{CURRENT_SOURCE_LOCATION, VIOLATION, MODULE});

    sut.panic();

    sut.reset();
    EXPECT_FALSE(hasAnyError());
}

TEST_F(TestHandler_test, prepareJumpWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "df6356a6-9e9e-4ee3-8a7c-7eb68cfe2516");
    auto buf = sut.prepareJump();
    EXPECT_NE(buf, nullptr);
}

TEST_F(TestHandler_test, onlyOneJumpCanBePrepared)
{
    ::testing::Test::RecordProperty("TEST_ID", "45ad9ab9-0f79-4b7c-8e36-76da3067c0fd");
    auto buf1 = sut.prepareJump();
    auto buf2 = sut.prepareJump();
    EXPECT_NE(buf1, nullptr);
    EXPECT_EQ(buf2, nullptr);
}

void jump(TestHandler& handler, int& jmpValue)
{
    auto* buf = handler.prepareJump();
    // setjmp must be used in a control flow construct like if, while (UB otherwise)
    if (setjmp(*buf) != handler.jumpIndicator())
    {
        // regular control flow panics
        handler.panic();
    }
    else
    {
        // jumped after regular control flow
        jmpValue = handler.jumpIndicator();
    }
}

TEST_F(TestHandler_test, panicTriggersPreparedJump)
{
    ::testing::Test::RecordProperty("TEST_ID", "2d99e382-ed43-4357-86f2-ef8d70c6acd8");
    int jmpValue{0};
    std::thread t(jump, std::ref(sut), std::ref(jmpValue));

    if (!t.joinable())
    {
        FAIL();
    }

    t.join();

    EXPECT_TRUE(sut.hasPanicked());
    // jumpIndicator() is consistent with value of setjmp after panic
    EXPECT_EQ(jmpValue, TestHandler::jumpIndicator());
    EXPECT_NE(jmpValue, 0);
}

void noJump(TestHandler& handler, int& jmpValue)
{
    jmp_buf buf;
    // setjmp must be used in a control flow construct like if, while (UB otherwise)
    if (setjmp(buf) != handler.jumpIndicator())
    {
        // regular control flow panics
        handler.panic();
    }
    else
    {
        // jumped after regular control flow
        jmpValue = handler.jumpIndicator();
    }
}

// This checks that panic will not jump without proper setup by test code
// Note that this must happen outside of the TestHandler implementation due to
// limitations of setjmp.
TEST_F(TestHandler_test, panicDoesNotTriggerUnpreparedJump)
{
    ::testing::Test::RecordProperty("TEST_ID", "23004b9e-4ec9-4fe5-9312-54907ad05967");
    int jmpValue{0};
    std::thread t(noJump, std::ref(sut), std::ref(jmpValue));

    if (!t.joinable())
    {
        FAIL();
    }

    t.join();

    EXPECT_TRUE(sut.hasPanicked());
    EXPECT_NE(jmpValue, TestHandler::jumpIndicator());
    EXPECT_EQ(jmpValue, 0);
}

} // namespace
