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

#include "test.hpp"
#include <gtest/gtest.h>

// some dummy modules under test
#include "module_a/error_reporting.hpp"
#include "module_b/error_reporting.hpp"

#include "iceoryx_hoofs/testing/error_reporting/test_support.hpp"

namespace
{
using namespace ::testing;
using namespace iox::err;
using namespace iox::cxx;
using namespace iox::testing;

using MyErrorA = module_a::errors::Error;
using MyCodeA = module_a::errors::Code;

using MyErrorB = module_b::errors::Error;
using MyCodeB = module_b::errors::Code;

class ErrorReportingApi_test : public Test
{
  public:
    void SetUp() override
    {
        TestErrorHandler::instance().reset();
    }

    void TearDown() override
    {
    }
};

TEST_F(ErrorReportingApi_test, panic)
{
    ::testing::Test::RecordProperty("TEST_ID", "a55f00f1-c89d-4d4d-90ea-6ca510ad3942");
    auto f = []() { IOX_PANIC(); };

    runInTestThread(f);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingApi_test, panicWithMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "cfbaf43b-de11-4858-ab86-ae3ae3fac2fe");
    auto f = []() { IOX_PANIC("message"); };

    runInTestThread(f);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingApi_test, reportNonFatal)
{
    ::testing::Test::RecordProperty("TEST_ID", "408a30b5-2764-4792-a5c6-97bff74f8902");
    auto f = []() { IOX_REPORT(MyCodeA::OutOfBounds, RUNTIME_ERROR); };

    runInTestThread(f);

    ASSERT_NO_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingApi_test, reportFatal)
{
    ::testing::Test::RecordProperty("TEST_ID", "a65c28fb-8cf6-4b9b-96b9-079ee9cb6b88");
    auto f = []() { IOX_REPORT_FATAL(MyCodeA::OutOfBounds); };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingApi_test, reportConditionallyTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "d95fe843-5e1b-422f-bd15-a791b639b43e");
    auto f = []() { IOX_REPORT_IF(true, MyCodeA::OutOfBounds, FATAL); };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingApi_test, reportConditionallyFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d9d6464-4586-4382-8d5f-38f3795af791");
    auto f = []() { IOX_REPORT_IF(false, MyCodeA::Unknown, FATAL); };

    runInTestThread(f);

    ASSERT_NO_PANIC();
    EXPECT_NO_ERROR();
}

TEST_F(ErrorReportingApi_test, requireTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c684878-20f8-426f-bb8b-7576b567d04f");
    auto f = []() { IOX_REQUIRE(true, MyCodeA::OutOfBounds); };

    runInTestThread(f);

    ASSERT_NO_PANIC();
    EXPECT_NO_ERROR();
}

TEST_F(ErrorReportingApi_test, requireFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb62d315-8854-401b-82af-6161ae45a34e");
    auto f = []() { IOX_REQUIRE(false, MyCodeA::OutOfBounds); };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingApi_test, checkPreconditionTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb6e2122-7c57-4657-9567-ecb63e26a3ed");
    auto f = [](int x) { IOX_PRECONDITION(x > 0); };

    runInTestThread(f, 1);

    ASSERT_NO_PANIC();
    EXPECT_NO_ERROR();
}

TEST_F(ErrorReportingApi_test, checkPreconditionFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2d27f6d-d0c7-405a-afbf-bf8a72661b20");
    auto f = [](int x) { IOX_PRECONDITION(x > 0); };

    runInTestThread([&]() { f(0); });

    runInTestThread(f, 0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingApi_test, checkAssumptionTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "a76ce780-3387-4ae8-8e4c-c96bdb8aa753");
    auto f = [](int x) { IOX_ASSUME(x > 0); };

    runInTestThread(f, 1);

    ASSERT_NO_PANIC();
    EXPECT_NO_ERROR();
}

TEST_F(ErrorReportingApi_test, checkAssumptionFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ee71bd3-9004-4950-8441-25e98cf8409c");
    auto f = [](int x) { IOX_ASSUME(x > 0); };

    runInTestThread(f, 0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingApi_test, checkPreconditionWithMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "18d5b9a6-2d60-478e-8c50-d044a3672290");

    auto f = [](int x) { IOX_PRECONDITION(x > 0, "some message"); };

    runInTestThread(f, 0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingApi_test, checkAssumptionWithMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "b416674a-5861-4ab7-947b-0bd0af2f627b");
    auto f = [](int x) { IOX_ASSUME(x > 0, "some message"); };

    runInTestThread(f, 0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingApi_test, reportErrorsFromDifferentModules)
{
    ::testing::Test::RecordProperty("TEST_ID", "5bc53c41-4e4b-466e-b706-603ed5a3d0cf");
    auto f = []() {
        IOX_REPORT(MyCodeA::OutOfBounds, RUNTIME_ERROR);
        IOX_REPORT(MyCodeB::OutOfMemory, RUNTIME_ERROR);
    };

    runInTestThread(f);

    ASSERT_NO_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
    ASSERT_ERROR(MyCodeB::OutOfMemory);
}

TEST_F(ErrorReportingApi_test, panicAtUnreachableCode)
{
    ::testing::Test::RecordProperty("TEST_ID", "54e84082-42eb-4fd3-af30-2647f9616719");
    auto f = []() { IOX_UNREACHABLE(); };

    runInTestThread(f);

    ASSERT_PANIC();
}

} // namespace
