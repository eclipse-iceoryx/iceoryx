// must be defined globally before first inclusion (actually from cmake, preferably)
#include <gtest/gtest.h>
#define TEST_PLATFORM // override the error handling for testing purposes
#define DEBUG         // IOX_DEBUG_ASSERT is active

#include "test.hpp"
#include <gtest/gtest-death-test.h>

#include "iceoryx_hoofs/error_handling_3/api.hpp"

// some dummy modules under test
#include "iceoryx_hoofs/error_handling_3/modules/module_a/errors.hpp"

#include "iceoryx_hoofs/cxx/optional.hpp"

#include <iostream>

#include "iceoryx_hoofs/cxx/expected.hpp"

namespace
{
using namespace ::testing;
using namespace eh3;
using std::cout;
using std::endl;

using ErrorA = module_a::error::Error;
using CodeA = module_a::error::ErrorCode;

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

int f1(int x)
{
    IOX_PRECOND(x > 0);
    return x;
}

TEST_F(ErrorHandlingUseCase_test, case1)
{
    int in = 73;
    auto out = f1(in);
    EXPECT_EQ(in, out);
}

TEST_F(ErrorHandlingUseCase_test, case2)
{
    int in = -73;
    auto out = f1(in);
    EXPECT_EQ(in, out);
}

} // namespace
