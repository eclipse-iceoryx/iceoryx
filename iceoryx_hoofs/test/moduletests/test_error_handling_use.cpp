// must be defined globally before first inclusion (actually from cmake, preferably)
#include "iceoryx_hoofs/error_handling_3/platform/error_kind.hpp"
#include "iceoryx_hoofs/error_handling_3/platform/test_platform/error_reporting.hpp"
#include <gtest/gtest.h>
#define TEST_PLATFORM // override the error handling for testing purposes
#define DEBUG         // defensive checks (DEBUG_ASSERT, PRECOND) are active

#include "test.hpp"
#include <gtest/gtest-death-test.h>

#include "iceoryx_hoofs/error_handling_3/api.hpp"

// some dummy modules under test
#include "iceoryx_hoofs/error_handling_3/modules/module_a/errors.hpp"

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"

#include <iostream>

namespace
{
using namespace ::testing;
using namespace eh3;
using namespace iox::cxx;
using std::cout;
using std::endl;

using Error = module_a::error::Error;
using Code = module_a::error::ErrorCode;

struct AnotherError
{
    AnotherError(Error& error)
        : m_error(error)
    {
    }

    Error m_error;
};

#define IGNORE(expr) (void)(expr)

#define ASSERT_NO_PANIC()                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_FALSE(eh3::hasPanicked());                                                                              \
    } while (false)

#define ASSERT_PANIC()                                                                                                 \
    do                                                                                                                 \
    {                                                                                                                  \
        ASSERT_TRUE(eh3::hasPanicked());                                                                               \
    } while (false)

class ErrorHandlingUseCase_test : public Test
{
  public:
    void SetUp() override
    {
        eh3::resetPanic();
    }

    void TearDown() override
    {
    }
};

// Use case: precondition failure that indicates a bug
// Reaction
// panic if not satisfied
int f1(int x)
{
    IOX_EXPECTS(x > 0);
    return x;
}

// Use case: recoverable error occurs during call
// Reaction
// 1. generate error
// 2. report error
// 3. propagate error - different control flow
// NB: optional, expected, result etc. are all possible
// NB: we cannot throw here if we want to continue with the same control flow
// Optional message in framework
expected<int, Error> f2(int x)
{
    // assume preconditions are OK
    // do something

    if (x <= 0)
    {
        // create recoverable error
        auto err = IOX_ERROR(Code::Unknown);

        // alternatively this could provide us the error as return
        IOX_REPORT(err, RUNTIME_ERROR);

        // expected is fairly inconvenient due to lack of conversions etc.
        // this can be fixed
        return error<Error>(err);
    }

    return success<int>(x);
}

// Use case: recoverable error occurs during call of another function
// Reaction
// 1. report error
// 2. propagate or handle error - different control flow
// NB: we cannot throw here if we want to continue with the same control flow
// TODO: general transform case (other error types)
expected<int, AnotherError> f3(int x)
{
    // assume preconditions are OK

    auto y = f2(x);
    if (y.has_error())
    {
        // optional report
        // TODO: overload for expected
        auto err = y.get_error();
        IOX_REPORT(err, RUNTIME_ERROR);

        // transform error (transformation must exist)
        return error<AnotherError>(AnotherError(err));
        // cannot deduce argument as is
        //   bad - return error(AnotherError(err));
    }

    // verbose expected syntax
    return success<int>(y.value());

    // cannot deduce argument as is or at least ok(y.value());
    //  return y.value();
}

// Use case: non-recoverable error occurs
// Reaction
// 1. report fatal error
// 2. panic - do not return
// NB: we could throw here as we will not continue, but this makes
// only sense if we do not terminate
// NB: no bug, not to be used for precondition failure
int f4(int x)
{
    // preconditions are OK
    // do something and encounter fatal condition

    if (x <= 0)
    {
        // IOX_REPORT(Code::OutOfMemory, FATAL);
        //   or
        //   NB: Simon prefers
        //  Bob, IOX_REPORT_FATAL() noreturn
        IOX_FATAL(Code::OutOfMemory);
    }

    return x;
}

// Use case: check for non-recoverable error conditions
// Reaction
// 1. if condition is not satisfied report fatal error
// 2. panic - do not return
int f5(int x)
{
    // preconditions are OK

    // Bob: IOX_ENSURE

    IOX_REQUIRE(x > 0, Code::OutOfMemory);

    // syntactic sugar for (arguably more clear)

    // Bob: not needed
    // IOX_REPORT_IF(!(x > 0), Code::OutOfMemory, FATAL);

    return x;
}

// Use case: errors that are not supposed to happen (defensive checks against bugs)
// Reaction
// Only if enabled, nothing happens otherwise
// 1. if condition is not satisfied report fatal error
// 2. panic - do not return
// NB: preconditions are a similar category and checks can be disabled for performance
// NB: failure indicates a bug
// TODO: do we want the debug and regular assert? at least the debug IMO
//    do we want a version for postcodnitions, likely without a specific error as a postcondition
//    violation is a bug
int f6(int x)
{
    // preconditions are OK but there is a problem (bug) in the function body

    // supposed to be used for postconditions or mid-function
    // Matthias: prefer string message, rename ASSERT if not used elsewhere
    IOX_DEBUG_ASSERT(x > 0);

    return x;
}

// Use case: terminate without specific error
// Reaction
// 1. panic - do not return
// NB: should not be used in most cases where a concrete error is desired
int f7(int x)
{
    // preconditions are OK

    if (x <= 0)
    {
        IOX_PANIC("panic!!!");
        // or
        IOX_PANIC();
    }

    return x;
}

// Use case: recoverable error occurs during call of another function
// Reaction but is deemed irecoverable later
// 1. report error
// 2. propagate or handle error - different control flow
// NB: we cannot throw here if we want to continue with the same control flow
// TODO: general transform case (other error types)
int f8(int x)
{
    // assume preconditions are OK
    auto y = f2(x);

    /*
        if (y.has_error())
        {
            // optional report
            // TODO: overload for expected
            IOX_FATAL(y.get_error());

            // should not be reachable
            return 0;
        }
    */
    // or shorter
    IOX_REQUIRE(!y.has_error(), y.get_error());
    // would have to guarantee noreturn in case of failure
    // could use continuations like or_else() but this could not directly return, only execute something

    // return y.value();
    return 0;
}

// Use case: check for non-recoverable error conditions
// Reaction
// 1. if condition is not satisfied report fatal error
// 2. panic - do not return
// similar to f5 but explicitly generated error exists
int f9(int x)
{
    // preconditions are OK

    auto err = IOX_ERROR(Code::OutOfMemory);
    IOX_REQUIRE(x > 0, err);

    return x;
}

// ***Correct use (no bug) scenarios***

TEST_F(ErrorHandlingUseCase_test, unconditionalPanic)
{
    int in = 0;
    auto out = f7(in);
    IGNORE(out);

    ASSERT_PANIC();
}

TEST_F(ErrorHandlingUseCase_test, expectedSuccess)
{
    int in = 73;
    auto out = f2(in);
    ASSERT_TRUE(!out.has_error());
    ASSERT_NO_PANIC();

    EXPECT_EQ(in, out.value());
}

TEST_F(ErrorHandlingUseCase_test, expectedFailure)
{
    int in = 0;
    auto out = f2(in);
    EXPECT_TRUE(out.has_error());

    ASSERT_NO_PANIC();
}

TEST_F(ErrorHandlingUseCase_test, internalCallFailure)
{
    int in = 0;
    auto out = f3(in);
    EXPECT_TRUE(out.has_error());

    ASSERT_NO_PANIC();
}

TEST_F(ErrorHandlingUseCase_test, fatalError)
{
    int in = 0;
    auto out = f4(in);
    IGNORE(out);

    ASSERT_PANIC();
}

TEST_F(ErrorHandlingUseCase_test, requireFailure)
{
    int in = 0;
    auto out = f5(in);
    IGNORE(out);

    ASSERT_PANIC();
}

TEST_F(ErrorHandlingUseCase_test, internalCallFatalFailure)
{
    int in = 0;
    auto out = f8(in);
    IGNORE(out);

    ASSERT_PANIC();
}


TEST_F(ErrorHandlingUseCase_test, requireFailure2)
{
    int in = 0;
    auto out = f9(in);
    IGNORE(out);

    ASSERT_PANIC();
}

// *** Bug-checking/defensive scenarios***

TEST_F(ErrorHandlingUseCase_test, preconditionSatisfied)
{
    int in = 73;
    auto out = f1(in);
    ASSERT_NO_PANIC();

    EXPECT_EQ(in, out);
}

TEST_F(ErrorHandlingUseCase_test, preconditionViolated)
{
    int in = 0;
    auto out = f1(in);
    IGNORE(out);

    ASSERT_PANIC();
}

TEST_F(ErrorHandlingUseCase_test, debugAssertFailure)
{
    int in = 0;
    auto out = f6(in);
    IGNORE(out);

    ASSERT_PANIC();
}

} // namespace
