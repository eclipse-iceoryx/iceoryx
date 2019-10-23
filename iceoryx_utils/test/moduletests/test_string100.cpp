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
#include "iceoryx_utils/fixed_string/string100.hpp"


using namespace ::testing;
using namespace iox::cxx;


class CString100_test : public Test
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

TEST_F(CString100_test, ReplaceWithBetterFixedString)
{
    const char v1[] = "hello world";
    std::string v2 = "hello world";
    CString100 a1(v1);
    CString100 a2(v2);
    CString100 a3 = CString100("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    a3 = a2;

    EXPECT_THAT(a1, Eq(a2));

    std::string foo = a1;
    EXPECT_THAT(foo, Eq("hello world"));
    EXPECT_THAT(strcmp(static_cast<const char*>(a2), "hello world"), Eq(0));
    EXPECT_THAT(a2 == a3, Eq(true));
}
