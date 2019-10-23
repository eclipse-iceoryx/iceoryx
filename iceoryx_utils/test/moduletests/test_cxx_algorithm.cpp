// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"
#include "iceoryx_utils/cxx/algorithm.hpp"

using namespace ::testing;
using namespace iox::algorithm;

class algorithm_test : public Test
{
  public:
    void SetUp()
    {
        internal::CaptureStderr();
    }
    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

TEST_F(algorithm_test, MaxOfOneElement)
{
    EXPECT_THAT(max(12.34f), Eq(12.34f));
}

TEST_F(algorithm_test, MaxOfTwoElements)
{
    EXPECT_THAT(max(56.78f, 12.34f), Eq(56.78f));
}

TEST_F(algorithm_test, MaxOfManyElements)
{
    EXPECT_THAT(max(56.78f, 33.44f, 12.34f, -0.1f, 5.5f, 10001.f), Eq(10001.f));
}

TEST_F(algorithm_test, MinOfOneElement)
{
    EXPECT_THAT(min(0.0123f), Eq(0.0123f));
}

TEST_F(algorithm_test, MinOfTwoElements)
{
    EXPECT_THAT(min(0.0123f, -91.12f), Eq(-91.12f));
}

TEST_F(algorithm_test, MinOfManyElements)
{
    EXPECT_THAT(min(0.0123f, -91.12f, 123.92f, -1021.2f, 0.0f), Eq(-1021.2f));
}
