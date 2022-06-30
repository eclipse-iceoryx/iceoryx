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

using B_Error = module_B::Error;
using B_Code = module_B::ErrorCode;

using A_Error = module_A::Error;
using A_Code = module_A::ErrorCode;

static bool g_terminateCalled;

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

template <typename Code>
RuntimeError to_error(Code code)
{
    return RuntimeError::from_error(create_error(code));
}
#endif

class EH_test : public Test
{
  public:
    void SetUp()
    {
        // TODO this does not return so is not really useful to track terminate was called
        // (distinguish crash and terminate call - if not needed we can use death tests)

        g_terminateCalled = false;
        terminateHandler = std::set_terminate([]() { g_terminateCalled = true; });

#ifdef TEST_PLATFORM
        ErrorHandler::reset();
#endif
    }
    virtual void TearDown()
    {
        std::set_terminate(terminateHandler);
    }

    std::terminate_handler terminateHandler;
};

TEST_F(EH_test, fatalError)
{
    // when we just want to abort the program (gracefully)
    // equivalent to IOX_RAISE(FATAL, code);
    IOX_FATAL(A_Code::Unknown);
}

TEST_F(EH_test, raiseSpecific)
{
    // when we know and care about the specific error
    IOX_RAISE(WARNING, A_Code::OutOfBounds);
    IOX_RAISE(ERROR, A_Code::Unknown);
    IOX_RAISE(FATAL, A_Code::OutOfMemory);
}

TEST_F(EH_test, raiseFromDifferentModules)
{
    module_A::function();
    module_B::function();
}

TEST_F(EH_test, raiseConditionally)
{
    // shorthand notation
    int x = 11;
    IOX_RAISE_IF(x > 10, WARNING, A_Code::OutOfBounds);

    auto f = [] { return true; };
    IOX_RAISE_IF(f, FATAL, B_Code::OutOfMemory);
}

TEST_F(EH_test, assertCondition)
{
    // shorthand notation, always fatal
    int x = 10;
    IOX_ASSERT(x < 10, A_Code::OutOfBounds);

    auto f = [] { return false; };
    IOX_ASSERT(f, A_Code::OutOfMemory);
}

TEST_F(EH_test, debugAssert)
{
    // fatal but a NOOP in release mode (like assert but with
    // custom handling when active)
    IOX_DEBUG_ASSERT(false, A_Code::OutOfBounds);
}

TEST_F(EH_test, additionalOutput)
{
    // works with any macro but currently the underlying stream
    // is not exclusive stream for error handling (TODO)
    IOX_RAISE(FATAL, A_Code::OutOfMemory) << " additional error message " << 21 << "\n";
}

TEST_F(EH_test, conditionalAdditionalOutput)
{
    // add additional output if an error occurred
    IOX_RAISE_IF(true, ERROR, A_Code::OutOfBounds) << "this is printed\n";
    IOX_RAISE_IF(false, ERROR, A_Code::OutOfBounds) << "this is not\n";
}

TEST_F(EH_test, conditionalFunctionCall)
{
    // call some function custom arguments if an error occurred
    // syntactic sugar
    int x{0};
    auto f = [&](int a) { x = a; };

    IOX_RAISE_IF(true, ERROR, A_Code::OutOfBounds).IF_RAISED(f, 21);
    EXPECT_EQ(x, 21);

    IOX_RAISE_IF(false, ERROR, A_Code::OutOfBounds).IF_RAISED(f, 12);
    EXPECT_EQ(x, 21);
}

TEST_F(EH_test, fullFunctionality)
{
    int x = 10;
    int n = 0;
    auto f = [&](int a) { n += a; };

    IOX_RAISE_IF(x <= 10, ERROR, A_Code::OutOfBounds).IF_RAISED(f, 5) << "this is printed\n";
    IOX_RAISE_IF(x > 10, ERROR, A_Code::OutOfBounds).IF_RAISED(f, 3) << "this is not\n";

    EXPECT_EQ(n, 5);
}

// recovery proposoal (it is always possible to do it with conditionals in
// a straightfoward way)
TEST_F(EH_test, errorRecovery)
{
    using namespace iox::cxx;

    int x = 3;
    auto f = [](int) -> optional<int> { return nullopt; };
    optional<int> result = f(x); // try obtaining a result, which fails

    auto tryRecover1 = [&](int a) { result = f(a); }; // retry, but this will fail again
    auto tryRecover2 = [&](int a) { result = a; };    // try an alternative algorithm

    IOX_RAISE_IF(!result, ERROR, B_Code::Unknown).IF_RAISED(tryRecover1, x);
    IOX_RAISE_IF(!result, ERROR, B_Code::Unknown).IF_RAISED(tryRecover2, x);
    IOX_RAISE_IF(!result, FATAL, B_Code::Unknown) << "recovery failed";

    // can be made more elegant but already hides the branching
    // and we can simulate recovery blocks arguably in a more concise way
    // (performance should not be affected much if at all)

    ASSERT_TRUE(result);
    EXPECT_EQ(*result, x);
}


#ifdef TEST_PLATFORM
TEST_F(EH_test, verifyError1)
{
    // activate throwing behavior
    ErrorHandler::set(throwHandler());

    auto expectedError = to_error(B_Code::OutOfMemory);
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
TEST_F(EH_test, verifyError2)
{
    // activate throwing behavior
    ErrorHandler::set(throwHandler());

    auto expectedError = to_error(B_Code::OutOfMemory);
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
TEST_F(EH_test, verifyError3)
{
    // ErrorHandler::finalize(); // will abort if we try to set the handler afterwards
    // activate
    auto& handler = testHandler();
    ErrorHandler::set(handler);
    // set cannot fail so we know handler was set

    auto expectedError = to_error(B_Code::OutOfMemory);

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
