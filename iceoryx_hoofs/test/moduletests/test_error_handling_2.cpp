#include "iceoryx_hoofs/error_handling_2/module/config.hpp"

#include "iceoryx_hoofs/error_handling_2/api.hpp"

#include "iceoryx_hoofs/cxx/type_traits.hpp"

#include "test.hpp"

#include <iostream>

namespace
{
using namespace ::testing;
using std::cout;
using std::endl;

#if 1
TEST(EH_test, raise)
{
    // unspecific error
    IOX_RAISE(WARNING);

    IOX_RAISE(WARNING, A_Code::OutOfBounds);
    IOX_RAISE(FATAL, B_Code::OutOfMemory);

    auto a = GenericError(A_Code::OutOfBounds);
    auto b = GenericError(B_Code::OutOfBounds);
    EXPECT_EQ(a, a);
    EXPECT_NE(b, a);
}

TEST(EH_test, fatal)
{
    IOX_FATAL();
    IOX_FATAL(B_Code::OutOfMemory);
}

TEST(EH_test, raise_if)
{
    int x = 11;
    IOX_RAISE_IF(x > 10, WARNING, A_Code::OutOfBounds);
    IOX_RAISE_IF(x > 20, ERROR, A_Code::OutOfBounds);

    auto f = [] { return true; };
    IOX_RAISE_IF(f, FATAL);
}

TEST(EH_test, assert)
{
    int x = 11;
    IOX_ASSERT(x < 10, A_Code::OutOfBounds);
    IOX_ASSERT(x < 10);
}

TEST(EH_test, debug_assert)
{
    int x = 11;
    IOX_DEBUG_ASSERT(x < 10, A_Code::OutOfBounds) << "message";
    IOX_DEBUG_ASSERT(x < 10);
}

TEST(EH_test, msg)
{
    auto f = [](int x) { std::cout << "Buuuuh!" << x << std::endl; };

    IOX_RAISE_IF(true, WARNING, A_Code::OutOfBounds) << " Warning Panic\n";

    IOX_RAISE_IF(true, FATAL, A_Code::OutOfBounds).and_call(f, 73) << " Fatal Panic\n";

    // the semantics are not clear from the wording (it is only called in case of error)
    //
    IOX_RAISE(WARNING, A_Code::OutOfBounds);
    IOX_RAISE(WARNING, A_Code::OutOfBounds).and_call(f, 74);
    IOX_ASSERT(true).and_call(f, 75);

    // and_call will not work if this is opimized away naively
    IOX_DEBUG_ASSERT(false).and_call(f, 76);
}
#else


TEST(EH_test, debug_assert)
{
    IOX_RAISE(WARNING);
    IOX_RAISE(FATAL, B_Code::OutOfMemory) << "some error message\n";

    // int x = 0;

    // // need an empty proxy
    // IOX_DEBUG_ASSERT(x > 0) << "debug assert message\n";
}
#endif

#if 0
// currently we throw in the proxy dtor which will terminate,
// we need a trick to use the operator<< and handle the error after the output
TEST(EH_test, verify_error)
{
    auto expectedError = GenericError(B_Code::OutOfMemory);
    try
    {
        // calling f which raises multiple errors would be a problem
        // with the exception verification technique
        // but this can only happen if destructors raise errors
        // (which is bad, as with exceptions)
        IOX_RAISE(FATAL, B_Code::OutOfMemory);
    }
    // TODO: lacks elegance but works (with limitations of double exception)
    //       note: we can also check it with a testhandler that does not require
    //       exceptions
    catch (B_Error& e)
    {
        std::cout << "caught1 " << e.name() << " in module " << e.module() << std::endl;

        // we have no comparison operator for the concrete errors (but could have)
        EXPECT_EQ(expectedError, GenericError(e.module(), e.code()));
        return;
    }
    catch (GenericError& e)
    {
        // should not be needed if we know the concrete error
        std::cout << "caught2 " << e.code() << " in module " << e.module() << std::endl;
        EXPECT_EQ(expectedError, e);
        return;
    }
    catch (...)
    {
        // needed
        std::cout << "caught ..." << std::endl;
    }
    FAIL();
}
#endif
} // namespace