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
    auto f = []() { IOX_PANIC(); };

    f();

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, unconditionalPanicWithMessage)
{
    auto f = []() { IOX_PANIC("message"); };

    f();

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, reportNonFatal)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_REPORT(e, RUNTIME_ERROR);
    };

    f();

    ASSERT_NO_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportNonFatalByCode)
{
    auto f = []() { IOX_REPORT(MyCodeA::OutOfBounds, RUNTIME_ERROR); };

    f();

    ASSERT_NO_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingAPI_test, reportFatal)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_REPORT_FATAL(e);
    };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportFatalByCode)
{
    auto f = []() { IOX_REPORT_FATAL(MyCodeA::OutOfBounds); };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingAPI_test, reportConditionallyTrue)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_REPORT_IF(true, e, FATAL);
    };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportConditionallyByCode)
{
    auto f = []() { IOX_REPORT_IF(true, MyCodeA::OutOfBounds, FATAL); };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingAPI_test, reportConditionallyFalse)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_REPORT_IF(false, e, FATAL);
    };

    f();

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, assertTrue)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_ASSERT(true, e);
    };

    f();

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, assertFalse)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCodeA::Unknown);
        IOX_ASSERT(false, e);
    };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, assertByCode)
{
    auto f = []() { IOX_ASSERT(false, MyCodeA::OutOfBounds); };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
}

TEST_F(ErrorReportingAPI_test, checkPreconditionTrue)
{
    auto f = [](int x) { IOX_PRECONDITION(x > 0, ""); };

    f(1);

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkPreconditionFalse)
{
    auto f = [](int x) { IOX_PRECONDITION(x > 0, ""); };

    f(0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkAssumptionTrue)
{
    auto f = [](int x) { IOX_ASSUME(x > 0, ""); };

    f(1);

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkAssumptionFalse)
{
    auto f = [](int x) { IOX_ASSUME(x > 0, ""); };

    f(0);

    ASSERT_PANIC();
}


TEST_F(ErrorReportingAPI_test, checkPreconditionWithMessage)
{
    auto f = [](int x) { IOX_PRECONDITION(x > 0, "message"); };

    f(0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkAssumptionWithMessage)
{
    auto f = [](int x) { IOX_ASSUME(x > 0, "message"); };

    f(0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, reportExpectedAsError)
{
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

    g();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCodeA::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportErrorsFromDifferentModules)
{
    auto f = []() {
        IOX_REPORT(MyCodeA::OutOfBounds, RUNTIME_ERROR);
        IOX_REPORT(MyCodeB::OutOfMemory, RUNTIME_ERROR);
    };

    f();

    ASSERT_NO_PANIC();
    ASSERT_ERROR(MyCodeA::OutOfBounds);
    ASSERT_ERROR(MyCodeB::OutOfMemory);
}

} // namespace
