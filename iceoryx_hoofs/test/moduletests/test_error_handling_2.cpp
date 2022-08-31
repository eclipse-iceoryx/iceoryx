#include "iceoryx_hoofs/error_handling_2/runtime_error.hpp"
#include <gtest/gtest-death-test.h>
#define TEST_PLATFORM
#define DEBUG // IOX_DEBUG_ASSERT active?

#include "iceoryx_hoofs/error_handling_2/module/module_A.hpp"
#include "iceoryx_hoofs/error_handling_2/module/module_B.hpp"

#include "iceoryx_hoofs/cxx/optional.hpp"

#ifdef TEST_PLATFORM
#include "iceoryx_hoofs/error_handling_2/platform/test_platform/test_handler.hpp"
#endif

#include "test.hpp"

#include <exception>
#include <iostream>

namespace
{
using namespace ::testing;
using namespace eh;
using std::cout;
using std::endl;

using B_Error = module_b::Error;
using B_Code = module_b::ErrorCode;

using A_Error = module_a::Error;
using A_Code = module_a::ErrorCode;

#ifdef TEST_PLATFORM

// ********************error test utility*********************

// initialize handlers on first use
TestHandler& testHandler()
{
    static TestHandler h;
    return h;
}

ThrowHandler& throwHandler()
{
    static ThrowHandler h;
    return h;
}

// TODO: these can be test utilities in a separate header but will depend on gtest
template <typename Code, typename Level>
RuntimeError toError(Code code, Level level)
{
    return RuntimeError::from(createError(code), level);
}
#endif

uint32_t countError(const RuntimeError& error)
{
    return testHandler().errors().count(error);
}

bool terminationRequested()
{
    return testHandler().terminationRequested();
}

template <typename Code, typename Level>
bool expectError(Code code, Level level, uint32_t count = 1)
{
    auto error = toError(code, level);
    bool ret = !terminationRequested() && countError(error) == count;
    testHandler().reset();
    return ret;
}

template <typename Code>
bool expectFatalError(Code code, uint32_t count = 1)
{
    auto error = toError(code, FATAL);
    bool ret = terminationRequested() && countError(error) == count;
    testHandler().reset();
    return ret;
}

// debatable, in general we should not expect errors by default
bool expectNoError()
{
    bool ret = testHandler().errors().size() == 0;
    testHandler().reset();
    return ret;
}

// macros, to get line numbers
// can only deal with one error (count = 1, which could be extended)
#define EXPECT_ERROR(code, level) EXPECT_TRUE(expectError(code, level))
#define EXPECT_FATAL_ERROR(code) EXPECT_TRUE(expectFatalError(code))
#define EXPECT_NO_ERROR() EXPECT_TRUE(expectNoError())

// ******************************************************

class ErrorHandling_test : public Test
{
  public:
    void SetUp() override
    {
        testHandler().reset();
        ErrorHandler::set(testHandler());
    }

    void TearDown() override
    {
        std::set_terminate(terminateHandler);
    }

    std::terminate_handler terminateHandler;
};

TEST_F(ErrorHandling_test, fatalError)
{
    IOX_FATAL(A_Code::Unknown);

    EXPECT_FATAL_ERROR(A_Code::Unknown);
}

TEST_F(ErrorHandling_test, raiseSpecific)
{
    IOX_RAISE(WARNING, A_Code::OutOfBounds);
    EXPECT_ERROR(A_Code::OutOfBounds, WARNING);

    IOX_RAISE(ERROR, A_Code::Unknown);
    EXPECT_ERROR(A_Code::Unknown, ERROR);

    IOX_RAISE(FATAL, A_Code::OutOfMemory);
    EXPECT_FATAL_ERROR(A_Code::OutOfMemory);
}

TEST_F(ErrorHandling_test, raiseFromDifferentModules)
{
    module_a::function();
    EXPECT_ERROR(A_Code::OutOfBounds, ERROR);

    module_b::function();
    EXPECT_FATAL_ERROR(B_Code::OutOfMemory);
}

TEST_F(ErrorHandling_test, raiseConditionally)
{
    // shorthand notation
    int x = 11;
    IOX_RAISE_IF(x > 10, WARNING, A_Code::OutOfBounds);
    EXPECT_ERROR(A_Code::OutOfBounds, WARNING);

    auto f = [] { return true; };
    IOX_RAISE_IF(f, FATAL, B_Code::OutOfMemory);
    EXPECT_FATAL_ERROR(B_Code::OutOfMemory);
}

TEST_F(ErrorHandling_test, assertCondition)
{
    // shorthand notation, always fatal
    int x = 10;
    IOX_ASSERT(x < 10, A_Code::OutOfBounds);
    EXPECT_FATAL_ERROR(A_Code::OutOfBounds);

    IOX_ASSERT(false, A_Code::OutOfMemory);
    EXPECT_FATAL_ERROR(A_Code::OutOfMemory);
}

TEST_F(ErrorHandling_test, debugAssert)
{
    // fatal but a NOOP in release mode (like assert but with
    // custom handling when active)
    IOX_DEBUG_ASSERT(false, A_Code::OutOfBounds);
    EXPECT_FATAL_ERROR(A_Code::OutOfBounds);
}

TEST_F(ErrorHandling_test, additionalOutput)
{
    // works with any macro but currently the underlying stream
    // is not exclusive stream for error handling (TODO)
    IOX_RAISE(FATAL, A_Code::OutOfMemory) << " additional error message " << 21 << "\n";

    // cannot check the log output without mock
    EXPECT_FATAL_ERROR(A_Code::OutOfMemory);
}

TEST_F(ErrorHandling_test, conditionalAdditionalOutput)
{
    // add additional output if an error occurred
    IOX_RAISE_IF(true, ERROR, A_Code::OutOfBounds) << "this is printed\n";
    EXPECT_ERROR(A_Code::OutOfBounds, ERROR);

    IOX_RAISE_IF(false, ERROR, A_Code::OutOfBounds) << "this is not\n";
    EXPECT_NO_ERROR();
}

TEST_F(ErrorHandling_test, conditionalFunctionCall)
{
    // call some function custom arguments if an error occurred
    // syntactic sugar
    int x{0};
    auto f = [&](int a) { x = a; };

    IOX_RAISE_IF(true, ERROR, A_Code::OutOfBounds).onError(f, 21);
    EXPECT_ERROR(A_Code::OutOfBounds, ERROR);
    EXPECT_EQ(x, 21);

    IOX_RAISE_IF(false, ERROR, A_Code::OutOfBounds).onError(f, 12);
    EXPECT_NO_ERROR();
    EXPECT_EQ(x, 21);
}

TEST_F(ErrorHandling_test, fullFunctionality)
{
    int x = 10;
    int n = 0;
    auto f = [&](int a) { n += a; };

    IOX_RAISE_IF(x <= 10, ERROR, A_Code::OutOfBounds).onError(f, 5) << "this is printed\n";
    EXPECT_ERROR(A_Code::OutOfBounds, ERROR);

    IOX_RAISE_IF(x > 10, ERROR, A_Code::OutOfBounds).onError(f, 3) << "this is not\n";
    EXPECT_NO_ERROR();
    EXPECT_EQ(n, 5);
}

TEST_F(ErrorHandling_test, errorRecovery)
{
    using namespace iox::cxx;

    int x = 3;
    auto f = [](int) -> optional<int> { return nullopt; };
    optional<int> result = f(x); // try obtaining a result, which fails

    auto tryRecover1 = [&](int a) { result = f(a); }; // retry, but this will fail again
    auto tryRecover2 = [&](int a) { result = a; };    // try an alternative algorithm

    IOX_RAISE_IF(!result, ERROR, B_Code::Unknown).onError(tryRecover1, x);
    EXPECT_ERROR(B_Code::Unknown, ERROR);

    IOX_RAISE_IF(!result, ERROR, B_Code::Unknown).onError(tryRecover2, x);
    EXPECT_ERROR(B_Code::Unknown, ERROR);

    IOX_RAISE_IF(!result, FATAL, B_Code::Unknown) << "recovery failed";
    EXPECT_NO_ERROR();

    // can be made more elegant but already hides the branching
    // and we can simulate recovery blocks arguably in a more concise way
    // (performance should not be affected much if at all)

    ASSERT_TRUE(result);
    EXPECT_EQ(*result, x);
}

TEST_F(ErrorHandling_test, setHandlerAfterFinalizeTerminates)
{
    std::terminate_handler h = std::set_terminate([]() { std::cerr << "TERMINATE WILL BE CALLED"; });

    auto f = []() {
        ErrorHandler::finalize();
        ErrorHandler::set(throwHandler());
    };

    EXPECT_DEATH(f(), "TERMINATE WILL BE CALLED");

    std::set_terminate(h);
}

TEST_F(ErrorHandling_test, verifyMultipleErrors)
{
    auto& handler = testHandler();
    ErrorHandler::set(handler);

    auto expectedError = toError(B_Code::OutOfMemory, FATAL);

    EXPECT_EQ(countError(expectedError), 0);
    EXPECT_NO_ERROR();

    // multiple errors without termination
    IOX_RAISE(FATAL, B_Code::OutOfMemory);
    IOX_RAISE(FATAL, B_Code::OutOfMemory);
    IOX_RAISE(FATAL, A_Code::OutOfMemory);

    EXPECT_EQ(countError(expectedError), 2);

    // macro does not support count (and probably should not for brevity), use function
    expectFatalError(B_Code::OutOfMemory, 2);

    // counts were reset by check
    EXPECT_EQ(countError(expectedError), 0);
    EXPECT_NO_ERROR();
}

TEST_F(ErrorHandling_test, verifyErrorByThrowing)
{
    // activate throwing behavior
    ErrorHandler::set(throwHandler());

    auto expectedError = toError(B_Code::OutOfMemory, FATAL);
    try
    {
        // calling f which raises multiple errors would be a problem
        // with the exception verification technique
        // but this can only happen if destructors raise errors which is
        // forbidden (as with exceptions)
        IOX_RAISE(FATAL, B_Code::OutOfMemory);
    }
    catch (RuntimeError& e)
    {
        EXPECT_EQ(expectedError, e);
        return;
    }
    FAIL();
}

TEST_F(ErrorHandling_test, verifyErrorByRethrowing)
{
    // activate throwing behavior
    ErrorHandler::set(throwHandler());

    auto expectedError = toError(B_Code::OutOfMemory, FATAL);
    EXPECT_THROW(
        {
            try
            {
                IOX_RAISE(FATAL, B_Code::OutOfMemory);
            }
            catch (const RuntimeError& e)
            {
                EXPECT_EQ(expectedError, e);
                throw;
            }
        },
        RuntimeError);
}

} // namespace
