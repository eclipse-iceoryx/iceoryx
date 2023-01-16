// must be defined globally before first inclusion (actually from cmake, preferably)
#define TEST_PLATFORM // override the error handling for testing purposes
#define DEBUG // IOX_DEBUG_ASSERT is active

#include "test.hpp"
#include <gtest/gtest-death-test.h>

#include "iceoryx_hoofs/error_handling_3/api.hpp"

// some dummy modules under test
#include "iceoryx_hoofs/error_handling_3/modules/module_a/errors.hpp"
#include "iceoryx_hoofs/error_handling_3/modules/module_b/errors.hpp"


// convenience to check for errors
// must be included after modules that define the errors!
// (not a huge issue, but can this be improved?)
// #include "iceoryx_hoofs/testing/error_checking_3.hpp"

#include "iceoryx_hoofs/cxx/optional.hpp"

#include <iostream>

namespace
{
using namespace ::testing;
using namespace eh3;
using std::cout;
using std::endl;

using ErrorA = module_a::error::Error;
using CodeA = module_a::error::ErrorCode;

using ErrorB = module_b::error::Error;
using CodeB = module_b::error::ErrorCode;


class ErrorHandling3_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(ErrorHandling3_test, error_generation_works)
{
    auto err1 = module_a::error::OutOfBoundsError(); // specific
    auto err2 = toError(CodeA::OutOfBounds);         // factory from code
    auto err3 = ErrorA(CodeA::OutOfBounds);          // from code

    EXPECT_TRUE(equals(err1, err2));
    EXPECT_TRUE(equals(err2, err3));
}

TEST_F(ErrorHandling3_test, errors_from_different_modules_differ)
{
    auto ea1 = module_a::error::OutOfBoundsError();
    auto ea2 = toError(CodeA::OutOfBounds);
    auto ea3 = ErrorA(CodeA::OutOfBounds);

    auto eb1 = module_b::error::OutOfBoundsError();
    auto eb2 = toError(CodeB::OutOfBounds);
    auto eb3 = ErrorB(CodeB::OutOfBounds);

    EXPECT_FALSE(equals(ea1, eb1));
    EXPECT_FALSE(equals(ea2, eb2));
    EXPECT_FALSE(equals(ea3, eb3));
}

TEST_F(ErrorHandling3_test, direct_proxy_use_works)
{
    auto err = module_a::error::OutOfBoundsError();
    auto proxy = createProxy(CURRENT_SOURCE_LOCATION, FATAL, err);
}

TEST_F(ErrorHandling3_test, report_api_works)
{
    auto err = module_a::error::OutOfBoundsError();
    IOX_REPORT(err, FATAL);
    IOX_REPORT(CodeA::OutOfBounds, FATAL);
    IOX_REPORT(CodeB::OutOfBounds, FATAL);
}

TEST_F(ErrorHandling3_test, fatal_api_works)
{
    auto err = module_a::error::OutOfBoundsError();
    IOX_FATAL(err);
    IOX_FATAL(CodeA::OutOfBounds);
    IOX_FATAL(CodeB::OutOfBounds);
}

TEST_F(ErrorHandling3_test, report_if_api_error_case_works)
{
    auto err = module_a::error::OutOfBoundsError();

    int x = 0;
    IOX_REPORT_IF(x == 0, err, FATAL);
    IOX_REPORT_IF(x == 0, CodeA::OutOfBounds, FATAL);
    IOX_REPORT_IF(x == 0, CodeB ::OutOfBounds, FATAL);
}

TEST_F(ErrorHandling3_test, report_if_api_nonerror_case_works)
{
    auto err = module_a::error::OutOfBoundsError();

    int x = 1;
    IOX_REPORT_IF(x == 0, err, FATAL);
    IOX_REPORT_IF(x == 0, CodeA::OutOfBounds, FATAL);
    IOX_REPORT_IF(x == 0, CodeB ::OutOfBounds, FATAL);
}

TEST_F(ErrorHandling3_test, assert_api_error_case_works)
{
    auto err = module_a::error::OutOfBoundsError();

    int x = 1;
    IOX_ASSERT(x == 0, err);
    IOX_ASSERT(x == 0, CodeA::OutOfBounds);
    IOX_ASSERT(x == 0, CodeB ::OutOfBounds);
}

TEST_F(ErrorHandling3_test, assert_api_nonerror_case_works)
{
    auto err = module_a::error::OutOfBoundsError();

    int x = 0;
    IOX_ASSERT(x == 0, err);
    IOX_ASSERT(x == 0, CodeA::OutOfBounds);
    IOX_ASSERT(x == 0, CodeB ::OutOfBounds);
}

TEST_F(ErrorHandling3_test, debug_assert_api_error_case_works)
{
    auto err = module_a::error::OutOfBoundsError();

    int x = 1;
    IOX_DEBUG_ASSERT(x == 0, err);
    IOX_DEBUG_ASSERT(x == 0, CodeA::OutOfBounds);
    IOX_DEBUG_ASSERT(x == 0, CodeB ::OutOfBounds);
}

TEST_F(ErrorHandling3_test, debug_assert_api_nonerror_case_works)
{
    auto err = module_a::error::OutOfBoundsError();

    int x = 0;
    IOX_DEBUG_ASSERT(x == 0, err);
    IOX_DEBUG_ASSERT(x == 0, CodeA::OutOfBounds);
    IOX_DEBUG_ASSERT(x == 0, CodeB ::OutOfBounds);
}

TEST_F(ErrorHandling3_test, panic_api_works)
{
    // do we want function syntax?
    IOX_PANIC;
}

TEST_F(ErrorHandling3_test, additional_messages_are_logged)
{
    errorStream().str(""); // clear, will use logger later
    // if it is intended to report the message, the error objects need to support that
    // this is purely for logging
    IOX_REPORT(CodeA::OutOfBounds, FATAL) << "Hello " << 73;
    std::cout << errorStream().str();
}

TEST_F(ErrorHandling3_test, non_required_levels_are_not_reported)
{
    IOX_REPORT(CodeA::OutOfBounds, WARNING);
}

} // namespace
