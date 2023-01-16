// must be defined globally before first inclusion (actually from cmake, preferably)
#include "iceoryx_hoofs/error_handling_3/platform/error_kind.hpp"
#include <gtest/gtest.h>
#define TEST_PLATFORM // override the error handling for testing purposes
#define DEBUG         // IOX_DEBUG_ASSERT is active

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

class ErrorHandlingUseCase_test : public Test
{
  public:
    void SetUp() override
    {
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
    IOX_PRECOND(x > 0);
    return x;
}

// Use case: recoverable error occurs during call
// Reaction
// 1. generate error
// 2. report error
// 3. propagate error - different control flow
// NB: optional, expected, result etc. are all possible
// NB: we cannot throw here if we want to continue with the same control flow
expected<int, Error> f2(int x)
{
    // assume preconditions are OK
    // do something

    if (x <= 0)
    {
        // create recoverable error
        auto err = IOX_ERROR(Code::Unknown);

        // alternatively this could provide us the error as return
        IOX_REPORT(err, ERROR);

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
expected<int, Error> f3(int x)
{
    // assume preconditions are OK

    auto y = f2(x);

    if (y.has_error())
    {
        IOX_REPORT(y.get_error(), ERROR);
    }
    return y;
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
        IOX_FATAL(Code::OutOfMemory);
    }

    return x;
}

// Use case: check for non-recoverable error conditions
// Reaction
// 1. if condition is not satisfied report fatal error
// 2. panic - do not return
// NB: syntactic sugar
// NB: usable for postconditions, but we can think of having a separate category
int f5(int x)
{
    // preconditions are OK

    IOX_ASSERT(x > 0, Code::OutOfMemory);

    return x;
}

// Use case: errors that are not supposed to happen (defensive checks against bugs)
// Reaction
// Only if enabled, nothing happens otherwise
// 1. if condition is not satisfied report fatal error
// 2. panic - do not return
// NB: preconditions are a similar category and checks can be disabled for performance
// NB: failure indicates a bug
int f6(int x)
{
    // preconditions are OK

    IOX_DEBUG_ASSERT(x > 0, Code::OutOfMemory);

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
        IOX_PANIC;
    }

    return x;
}

TEST_F(ErrorHandlingUseCase_test, preconditionSatisfied)
{
    int in = 73;
    auto out = f1(in);
    EXPECT_EQ(in, out);
}

TEST_F(ErrorHandlingUseCase_test, preconditionViolated)
{
    int in = 0;
    auto out = f1(in);
    EXPECT_EQ(in, out);
}

TEST_F(ErrorHandlingUseCase_test, expectedSuccess)
{
    int in = 73;
    auto out = f2(in);
    ASSERT_TRUE(!out.has_error());
    EXPECT_EQ(in, out.value());
}

TEST_F(ErrorHandlingUseCase_test, expectedFailure)
{
    int in = 0;
    auto out = f2(in);
    EXPECT_TRUE(out.has_error());
}

TEST_F(ErrorHandlingUseCase_test, internalCallFailure)
{
    int in = 0;
    auto out = f3(in);
    EXPECT_TRUE(out.has_error());
}

TEST_F(ErrorHandlingUseCase_test, fatalError)
{
    int in = 0;
    auto out = f4(in);
    EXPECT_EQ(in, out);
}

TEST_F(ErrorHandlingUseCase_test, assertFailure)
{
    int in = 0;
    auto out = f5(in);
    EXPECT_EQ(in, out);
}

TEST_F(ErrorHandlingUseCase_test, debugAssertFailure)
{
    int in = 0;
    auto out = f6(in);
    EXPECT_EQ(in, out);
}

TEST_F(ErrorHandlingUseCase_test, unconditionalPanic)
{
    int in = 0;
    auto out = f7(in);
    EXPECT_EQ(in, out);
}

} // namespace
