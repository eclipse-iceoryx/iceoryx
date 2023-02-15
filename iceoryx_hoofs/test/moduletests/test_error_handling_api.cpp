#define IOX_DEBUG // defensive checks (IOX_ASSUME, IOX_PRECONDITION) are active

#include "test.hpp"
#include <gtest/gtest.h>

// some dummy modules under test
#include "error_reporting/module_a/error_reporting.hpp"
#include "error_reporting/module_b/error_reporting.hpp"

#include "error_reporting/test_helper.hpp"

#include "iceoryx_hoofs/cxx/expected.hpp"

namespace
{
using namespace ::testing;
using namespace iox::err;
using namespace iox::cxx;
using namespace iox::testing;

using MyErrorA = module_a::errors::Error;
using MyCodeA = module_a::errors::ErrorCode;

using MyErrorB = module_b::errors::Error;
using MyCodeB = module_b::errors::ErrorCode;

class ErrorReportingAPI_test : public Test
{
  public:
    void SetUp() override
    {
        iox::err::TestErrorHandler::instance().reset();
    }

    void TearDown() override
    {
    }
};

TEST_F(ErrorReportingAPI_test, unconditionalPanic)
{
    ::testing::Test::RecordProperty("TEST_ID", "a55f00f1-c89d-4d4d-90ea-6ca510ad3942");
    auto f = []() { IOX_PANIC(); };

    runInTestThread(f);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, unconditionalPanicWithMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "cfbaf43b-de11-4858-ab86-ae3ae3fac2fe");
    auto f = []() { IOX_PANIC("message"); };

    runInTestThread(f);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, reportNonFatal)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0fc49dd-bc12-49d9-8f36-9f49ec1a796b");
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_REPORT(e, RUNTIME_ERROR);
    };

    runInTestThread(f);

    ASSERT_NO_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportNonFatalByCode)
{
    ::testing::Test::RecordProperty("TEST_ID", "408a30b5-2764-4792-a5c6-97bff74f8902");
    auto f = []() { IOX_REPORT(MyCodeA::OutOfBounds, RUNTIME_ERROR); };

    runInTestThread(f);

    ASSERT_NO_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingAPI_test, reportFatal)
{
    ::testing::Test::RecordProperty("TEST_ID", "b8272d4f-f1ab-4168-809b-1770acf054b3");
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_REPORT_FATAL(e);
    };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportFatalByCode)
{
    ::testing::Test::RecordProperty("TEST_ID", "a65c28fb-8cf6-4b9b-96b9-079ee9cb6b88");
    auto f = []() { IOX_REPORT_FATAL(MyCodeA::OutOfBounds); };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingAPI_test, reportConditionallyTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "8c1fa807-a1f6-4618-add5-6d7472c5c1dc");
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_REPORT_IF(true, e, FATAL);
    };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportConditionallyByCode)
{
    ::testing::Test::RecordProperty("TEST_ID", "d95fe843-5e1b-422f-bd15-a791b639b43e");
    auto f = []() { IOX_REPORT_IF(true, MyCodeA::OutOfBounds, FATAL); };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingAPI_test, reportConditionallyFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "9d9d6464-4586-4382-8d5f-38f3795af791");
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_REPORT_IF(false, e, FATAL);
    };

    runInTestThread(f);

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, assertTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "3c684878-20f8-426f-bb8b-7576b567d04f");
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_ASSERT(true, e);
    };

    runInTestThread(f);

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, assertFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb62d315-8854-401b-82af-6161ae45a34e");
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_ASSERT(false, e);
    };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, assertByCode)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c4f2e4e-8bd2-495d-ba99-2274e22868aa");
    auto f = []() { IOX_ASSERT(false, MyCodeA::OutOfBounds); };

    runInTestThread(f);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingAPI_test, checkPreconditionTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "bb6e2122-7c57-4657-9567-ecb63e26a3ed");
    auto f = [](int x) { IOX_PRECONDITION(x > 0, ""); };

    runInTestThread([&]() { f(1); });

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkPreconditionFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "b2d27f6d-d0c7-405a-afbf-bf8a72661b20");
    auto f = [](int x) { IOX_PRECONDITION(x > 0, ""); };

    runInTestThread([&]() { f(0); });

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkAssumptionTrue)
{
    ::testing::Test::RecordProperty("TEST_ID", "a76ce780-3387-4ae8-8e4c-c96bdb8aa753");
    auto f = [](int x) { IOX_ASSUME(x > 0, ""); };

    runInTestThread([&]() { f(1); });

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkAssumptionFalse)
{
    ::testing::Test::RecordProperty("TEST_ID", "9ee71bd3-9004-4950-8441-25e98cf8409c");
    auto f = [](int x) { IOX_ASSUME(x > 0, ""); };

    runInTestThread([&]() { f(0); });

    ASSERT_PANIC();
}


TEST_F(ErrorReportingAPI_test, checkPreconditionWithMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "18d5b9a6-2d60-478e-8c50-d044a3672290");
    auto f = [](int x) { IOX_PRECONDITION(x > 0, "message"); };

    runInTestThread([&]() { f(0); });

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkAssumptionWithMessage)
{
    ::testing::Test::RecordProperty("TEST_ID", "b416674a-5861-4ab7-947b-0bd0af2f627b");
    auto f = [](int x) { IOX_ASSUME(x > 0, "message"); };

    runInTestThread([&]() { f(0); });

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, reportExpectedAsError)
{
    ::testing::Test::RecordProperty("TEST_ID", "316a1641-6750-421b-a414-1ce858e45529");
    // this is not ideal but currently as good as it gets (?) with expected
    auto f = []() -> expected<int, MyErrorA> {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        return iox::cxx::error<MyErrorA>(e);
    };

    auto g = [&]() {
        auto res = f();
        ASSERT_TRUE(res.has_error());
        IOX_REPORT(res, FATAL);
    };

    runInTestThread(g);

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportErrorsFromDifferentModules)
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

} // namespace
