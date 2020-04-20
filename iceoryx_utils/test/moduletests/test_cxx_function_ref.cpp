// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "iceoryx_utils/cxx/function_ref.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::cxx;

class function_refTest : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
    uint8_t m_iter{0};
};

using function_refDeathTest = function_refTest;

TEST_F(function_refTest, CreateEmpty)
{
    function_ref<void()> sut;
    EXPECT_FALSE(sut);
}

TEST_F(function_refTest, CreateEmptyAndAssign)
{
    auto lambda = [] {};
    function_ref<void()> sut;
    sut = lambda;
    EXPECT_TRUE(sut);
}

TEST_F(function_refTest, CreateWithNullptr)
{
    function_ref<void()> sut(nullptr);
    EXPECT_FALSE(sut);
}

TEST_F(function_refDeathTest, UBCreateWithTempLambda)
{
    function_ref<void()> sut([&] { m_iter++; });
    /// @todo
    // EXPECT_EXIT(sut(), ::testing::KilledBySignal(SIGSEGV), "Segmentation fault");
}

TEST_F(function_refTest, CreateWithCapturingLambdaVoidVoid)
{
    auto lambda = [&] { m_iter++; };
    function_ref<void(void)> sut(lambda);
    sut();
    EXPECT_THAT(m_iter, Eq(1));
}

TEST_F(function_refTest, CreateWithLambdaIntVoid)
{
    auto lambda = [](void) -> int { return 42; };
    function_ref<int(void)> sut(lambda);
    EXPECT_THAT(sut(), Eq(42));
}

TEST_F(function_refTest, CreateWithLambdaIntInt)
{
    auto lambda = [](int var) -> int { return ++var; };
    function_ref<int(int)> sut(lambda);
    EXPECT_THAT(sut(m_iter), Eq(1));
}
