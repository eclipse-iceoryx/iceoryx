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

TEST(EH_test, raise)
{
    // unspecific error
    IOX_RAISE(WARNING);

    IOX_RAISE(WARNING, A_Code::OutOfBounds);
    IOX_RAISE(FATAL, B_Code::OutOfMemory);
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
    IOX_RAISE_IF(x > 20, WARNING, A_Code::OutOfBounds);

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
    IOX_DEBUG_ASSERT(x < 10, A_Code::OutOfBounds);
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

} // namespace