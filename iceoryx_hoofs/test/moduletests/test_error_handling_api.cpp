#include "iceoryx_hoofs/error_reporting/platform/default/error_handler.hpp"
#define IOX_DEBUG // defensive checks (DEBUG_ASSERT, PRECOND) are active

#include "test.hpp"
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

// must be defined globally before first inclusion (actually from cmake, preferably)
#include "iceoryx_hoofs/error_reporting/platform/error_reporting.hpp"

#include "iceoryx_hoofs/error_reporting/api.hpp"

// some dummy modules under test
#include "error_reporting_modules/module_a/errors.hpp"

#include "iceoryx_hoofs/cxx/expected.hpp"

#include <iostream>

namespace
{
using namespace ::testing;
using namespace iox::err;
using namespace iox::cxx;
using std::cout;
using std::endl;

using MyError = module_a::error::Error;
using MyCode = module_a::error::ErrorCode;

/// @todo move to some test support file

#define ASSERT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(iox::err::TestErrorHandler::instance().hasPanicked());                                            \
    } while (false)

#define ASSERT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(iox::err::TestErrorHandler::instance().hasPanicked());                                             \
    } while (false)

#define ASSERT_ERROR(code)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        auto e = error_code_t(code);                                                                                   \
        ASSERT_TRUE(iox::err::TestErrorHandler::instance().hasError(e));                                               \
    } while (false)

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
        auto e = IOX_ERROR(MyCode::Unknown);
        IOX_REPORT(e, RUNTIME_ERROR);
    };

    f();

    ASSERT_NO_PANIC();
    ASSERT_ERROR(MyCode::Unknown);
}

TEST_F(ErrorReportingAPI_test, reportFatal)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCode::Unknown);
        IOX_REPORT_FATAL(e);
    };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCode::Unknown);
}


TEST_F(ErrorReportingAPI_test, reportConditionallyTrue)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCode::Unknown);
        IOX_REPORT_IF(true, e, FATAL);
    };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCode::Unknown);
}


TEST_F(ErrorReportingAPI_test, reportConditionallyFalse)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCode::Unknown);
        IOX_REPORT_IF(false, e, FATAL);
    };

    f();

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, assertTrue)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCode::Unknown);
        IOX_ASSERT(true, e);
    };

    f();

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, assertFalse)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCode::Unknown);
        IOX_ASSERT(false, e);
    };

    f();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCode::Unknown);
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
    auto f = []() -> expected<int, MyError> {
        auto e = IOX_ERROR(MyCode::Unknown);
        return iox::cxx::error<MyError>(e);
    };

    auto g = [&]() {
        auto res = f();
        ASSERT_TRUE(res.has_error());
        IOX_REPORT(res, FATAL);
    };

    g();

    ASSERT_PANIC();
    ASSERT_ERROR(MyCode::Unknown);
}

} // namespace
