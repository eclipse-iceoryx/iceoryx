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

bool expectTerminate;

#ifdef TEST_PLATFORM
// optional handlers to be used in addition
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

template <typename Code, typename Level>
RuntimeError toError(Code code, Level level)
{
    return RuntimeError::from(createError(code), level);
}
#endif

class ErrorHandling_test : public Test
{
  public:
    void SetUp() override
    {
        expectTerminate = false;
        terminateHandler = std::set_terminate([]() { EXPECT_EQ(expectTerminate, true); });

#ifdef TEST_PLATFORM
        ErrorHandler::reset();
#endif
    }
    void TearDown() override
    {
        std::set_terminate(terminateHandler);
    }

    std::terminate_handler terminateHandler;
};

TEST_F(ErrorHandling_test, fatalError)
{
    expectTerminate = false;
    IOX_FATAL(A_Code::Unknown);
}

TEST_F(ErrorHandling_test, raiseSpecific)
{
    IOX_RAISE(WARNING, A_Code::OutOfBounds);
    IOX_RAISE(ERROR, A_Code::Unknown);
    IOX_RAISE(FATAL, A_Code::OutOfMemory);
}

TEST_F(ErrorHandling_test, raiseFromDifferentModules)
{
    module_a::function();
    module_b::function();
}

TEST_F(ErrorHandling_test, raiseConditionally)
{
    // shorthand notation
    int x = 11;
    IOX_RAISE_IF(x > 10, WARNING, A_Code::OutOfBounds);

    auto f = [] { return true; };
    IOX_RAISE_IF(f, FATAL, B_Code::OutOfMemory);
}

TEST_F(ErrorHandling_test, assertCondition)
{
    // shorthand notation, always fatal
    int x = 10;
    IOX_ASSERT(x < 10, A_Code::OutOfBounds);

    auto f = [] { return false; };
    IOX_ASSERT(f, A_Code::OutOfMemory);
}

TEST_F(ErrorHandling_test, debugAssert)
{
    // fatal but a NOOP in release mode (like assert but with
    // custom handling when active)
    IOX_DEBUG_ASSERT(false, A_Code::OutOfBounds);
}

TEST_F(ErrorHandling_test, additionalOutput)
{
    // works with any macro but currently the underlying stream
    // is not exclusive stream for error handling (TODO)
    IOX_RAISE(FATAL, A_Code::OutOfMemory) << " additional error message " << 21 << "\n";
}

TEST_F(ErrorHandling_test, conditionalAdditionalOutput)
{
    // add additional output if an error occurred
    IOX_RAISE_IF(true, ERROR, A_Code::OutOfBounds) << "this is printed\n";
    IOX_RAISE_IF(false, ERROR, A_Code::OutOfBounds) << "this is not\n";
}

TEST_F(ErrorHandling_test, conditionalFunctionCall)
{
    // call some function custom arguments if an error occurred
    // syntactic sugar
    int x{0};
    auto f = [&](int a) { x = a; };

    IOX_RAISE_IF(true, ERROR, A_Code::OutOfBounds).onError(f, 21);
    EXPECT_EQ(x, 21);

    IOX_RAISE_IF(false, ERROR, A_Code::OutOfBounds).onError(f, 12);
    EXPECT_EQ(x, 21);
}

TEST_F(ErrorHandling_test, fullFunctionality)
{
    int x = 10;
    int n = 0;
    auto f = [&](int a) { n += a; };

    IOX_RAISE_IF(x <= 10, ERROR, A_Code::OutOfBounds).onError(f, 5) << "this is printed\n";
    IOX_RAISE_IF(x > 10, ERROR, A_Code::OutOfBounds).onError(f, 3) << "this is not\n";

    EXPECT_EQ(n, 5);
}

// recovery proposoal (it is always possible to do it with conditionals in
// a straightfoward way)
TEST_F(ErrorHandling_test, errorRecovery)
{
    using namespace iox::cxx;

    int x = 3;
    auto f = [](int) -> optional<int> { return nullopt; };
    optional<int> result = f(x); // try obtaining a result, which fails

    auto tryRecover1 = [&](int a) { result = f(a); }; // retry, but this will fail again
    auto tryRecover2 = [&](int a) { result = a; };    // try an alternative algorithm

    IOX_RAISE_IF(!result, ERROR, B_Code::Unknown).onError(tryRecover1, x);
    IOX_RAISE_IF(!result, ERROR, B_Code::Unknown).onError(tryRecover2, x);
    IOX_RAISE_IF(!result, FATAL, B_Code::Unknown) << "recovery failed";

    // can be made more elegant but already hides the branching
    // and we can simulate recovery blocks arguably in a more concise way
    // (performance should not be affected much if at all)

    ASSERT_TRUE(result);
    EXPECT_EQ(*result, x);
}


#ifdef TEST_PLATFORM
TEST_F(ErrorHandling_test, verifyError1)
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

// alternative with EXPECT_THROW check and rethrow
TEST_F(ErrorHandling_test, verifyError2)
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

// alternative with custom bookkeeping of errors
TEST_F(ErrorHandling_test, verifyError3)
{
    // ErrorHandler::finalize(); // will abort if we try to set the handler afterwards
    // activate
    auto& handler = testHandler();
    ErrorHandler::set(handler);
    // set cannot fail so we know handler was set

    auto expectedError = toError(B_Code::OutOfMemory, FATAL);

    EXPECT_EQ(handler.errors().count(expectedError), 0);

    // multiple errors without termination
    IOX_RAISE(FATAL, B_Code::OutOfMemory);
    IOX_RAISE(FATAL, B_Code::OutOfMemory);
    IOX_RAISE(FATAL, A_Code::OutOfMemory);

    EXPECT_EQ(handler.errors().count(expectedError), 2);

    handler.reset();
    EXPECT_EQ(handler.errors().count(expectedError), 0);
}

#endif

} // namespace
