
// must be included before the /api.hpp (it contains the modules error codes)
#include "iceoryx_hoofs/error_handling_2/module/config.hpp"

#include "iceoryx_hoofs/error_handling_2/api.hpp"
#include "iceoryx_hoofs/error_handling_2/error.hpp"

#include "iceoryx_hoofs/cxx/type_traits.hpp"

#include "iceoryx_hoofs/cxx/optional.hpp"


#include "test.hpp"

#include <iostream>

namespace
{
using namespace ::testing;
using std::cout;
using std::endl;

using namespace eh;


// for now the tests check compilation only,
// demonstrate usage and output
// for automated checks we need the test platform to
// verify a specific error was raised etc.
// this is demonstrated later

TEST(EH_test, raiseUnspecific)
{
    // when we do not care about the specific error
    IOX_RAISE(WARNING);
    IOX_RAISE(ERROR);
    IOX_RAISE(FATAL);
}

TEST(EH_test, fatalError)
{
    // when we just want to abort the program (gracefully)
    // equivalent to IOX_RAISE(FATAL);
    IOX_FATAL();
    IOX_FATAL(A_Code::Unknown);
}

TEST(EH_test, raiseSpecific)
{
    // when we know and care about the specific error
    IOX_RAISE(WARNING, A_Code::OutOfBounds);
    IOX_RAISE(ERROR, A_Code::Unknown);
    IOX_RAISE(FATAL, A_Code::OutOfMemory);
}

TEST(EH_test, raiseFromDifferentModules)
{
    // e.g. for functions in posh which also use hoofs header
    IOX_RAISE(ERROR, A_Code::OutOfBounds);
    IOX_RAISE(FATAL, B_Code::OutOfMemory);
}

TEST(EH_test, raiseConditionally)
{
    // shorthand notation
    int x = 11;
    IOX_RAISE_IF(x > 10, WARNING, A_Code::OutOfBounds);

    auto f = [] { return true; };
    IOX_RAISE_IF(f, FATAL);
}

TEST(EH_test, assertCondition)
{
    // shorthand notation, always fatal
    int x = 10;
    IOX_ASSERT(x < 10, A_Code::OutOfBounds);

    auto f = [] { return false; };
    IOX_ASSERT(f, A_Code::OutOfMemory);
}

TEST(EH_test, debugAssert)
{
    // fatal but a NOOP in release mode (like assert but with
    // custom handling when active)
    IOX_DEBUG_ASSERT(false, A_Code::OutOfBounds);
}

TEST(EH_test, additionalOutput)
{
    // works with any macro but currently the underlying stream
    // is not exclusive stream for error handling (TODO)
    IOX_RAISE(FATAL, A_Code::OutOfMemory) << " addtional error message " << 21 << "\n";
}

TEST(EH_test, conditionalAdditionalOutput)
{
    // add additional output if an error occurred
    IOX_RAISE_IF(true, ERROR, A_Code::OutOfBounds) << "this is printed\n";
    IOX_RAISE_IF(false, ERROR, A_Code::OutOfBounds) << " this is not\n";
}

TEST(EH_test, conditionalFunctionCall)
{
    // call some function custom arguments if an error occurred
    // syntactic sugar
    int x{0};
    auto f = [&](int a) { x = a; };

    IOX_RAISE_IF(true, ERROR, A_Code::OutOfBounds).IF_ERROR(f, 21);
    EXPECT_EQ(x, 21);

    IOX_RAISE_IF(false, ERROR, A_Code::OutOfBounds).IF_ERROR(f, 12);
    EXPECT_EQ(x, 21);
}
// requires test platform to succeed (as otherwise nothing is thrown)
// TODO: lacks elegance but works with test platform handler
TEST(EH_test, verifyConcreteError)
{
    // we could check for the concrete error
    // but then it would require a comparison operator (in each module)
    auto expectedError = GenericError(B_Code::OutOfMemory);
    try
    {
        // calling f which raises multiple errors would be a problem
        // with the exception verification technique
        // but this can only happen if destructors raise errors which is
        // forbidden (as with exceptions)
        //
        IOX_RAISE(FATAL, B_Code::OutOfMemory);
    }
    catch (B_Error& e)
    {
        std::cout << "caught " << e.name() << " in module " << e.module() << std::endl;
        // we have no comparison operator for the concrete errors (but could have)
        EXPECT_EQ(expectedError, GenericError(e.module(), e.code()));
        return;
    }
#if 0
    catch (GenericError& e)
    {
        // should not be needed if we know the concrete error
        std::cout << "caught " << e.code() << " in module " << e.module() << std::endl;
        EXPECT_EQ(expectedError, e);
        return;
    }
#endif
    // the expected error was not thrown
    FAIL();
}

// recovery proposoal (it is always possible to do it with conditionals in
// a straightfoward way)
TEST(EH_test, errorRecovery)
{
    using namespace iox::cxx;

    int arg = 3;
    auto f = [](int) -> optional<int> { return iox::cxx::nullopt; };
    optional<int> result = f(arg); // try obtaining a result

    // compute the result
    auto tryRecover1 = [&](int) { result = f(arg); }; // retry, but this will fail again
    auto tryRecover2 = [&](int a) { result = a; };    // try an alternative algorithm

    IOX_RAISE_IF(!result, ERROR, B_Code::Unknown).IF_ERROR(tryRecover1, arg);
    IOX_RAISE_IF(!result, ERROR, B_Code::Unknown).IF_ERROR(tryRecover2, arg);
    IOX_RAISE_IF(!result, FATAL, B_Code::Unknown) << "recovery failed";

    // TODO: can be made more elegant but already hides the branching
    // and we can simulate recovery blocks arguably in a more concise way
    // (performance should not be affected much if at all)

    ASSERT_TRUE(result);
    EXPECT_EQ(*result, arg);
}
} // namespace