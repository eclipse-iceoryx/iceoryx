// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/internal/cxx/function.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::cxx;

namespace {


template<typename T>
using fixed_size_function = iox::cxx::function<T, 128>;
using signature = int(int);
using test_function = fixed_size_function<signature>;


class function_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(function_test, DefaultCtorCreatesNoCallable)
{
    test_function sut;
    EXPECT_FALSE(sut);
}

}