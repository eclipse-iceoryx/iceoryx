// must be defined globally before first inclusion (actually from cmake, preferably)
#include "iceoryx_hoofs/error_reporting/platform/error_kind.hpp"
#include "iceoryx_hoofs/error_reporting/platform/test_platform/error_reporting.hpp"
#include <gtest/gtest.h>
#define TEST_PLATFORM // override the error handling for testing purposes
#define IOX_DEBUG     // defensive checks (DEBUG_ASSERT, PRECOND) are active

#include "test.hpp"
#include <gtest/gtest-death-test.h>

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

#define IGNORE(expr) (void)(expr)

// TODO: more fine-grained error checks (override backend handler)

#define ASSERT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(hasPanicked());                                                                                   \
    } while (false)

#define ASSERT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(hasPanicked());                                                                                    \
    } while (false)

class ErrorReportingAPI_test : public Test
{
  public:
    void SetUp() override
    {
        resetPanic();
    }

    void TearDown() override
    {
    }
};

TEST_F(ErrorReportingAPI_test, unconditionalPanic)
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
}

TEST_F(ErrorReportingAPI_test, reportFatal)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCode::Unknown);
        IOX_REPORT_FATAL(e);
    };

    f();

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, reportConditionallyTrue)
{
    auto f = []() {
        auto e = IOX_ERROR(MyCode::Unknown);
        IOX_REPORT_IF(true, e, FATAL);
    };

    f();

    ASSERT_PANIC();
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

TEST_F(ErrorReportingAPI_test, checkPreconditionTrue)
{
    auto f = [](int x) { IOX_PRECONDITION(x > 0); };

    f(1);

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkPreconditionFalse)
{
    auto f = [](int x) { IOX_PRECONDITION(x > 0); };

    f(0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkAssumptionTrue)
{
    auto f = [](int x) { IOX_ASSUME(x > 0); };

    f(1);

    ASSERT_NO_PANIC();
}

TEST_F(ErrorReportingAPI_test, checkAssumptionFalse)
{
    auto f = [](int x) { IOX_ASSUME(x > 0); };

    f(0);

    ASSERT_PANIC();
}

TEST_F(ErrorReportingAPI_test, reportExpectedWithError)
{
    auto f = []() -> expected<int, MyError> {
        auto e = IOX_ERROR(MyCode::Unknown);
        return iox::cxx::error<MyError>(e);
    };

    auto res = f();
    ASSERT_TRUE(res.has_error());
    IOX_REPORT(res, FATAL);

    ASSERT_PANIC();
}

} // namespace
