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

#include "iceoryx_utils/cxx/type_traits.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::cxx;

TEST(TypeTraitsTest, IsInvocableResolvesToTrue)
{
    auto lambda = [](int foo) -> void { foo++; };
    auto sut = is_invocable<decltype(lambda), int>::value;
    EXPECT_TRUE(sut);
}

TEST(TypeTraitsTest, IsInvocableResolvesToFalse)
{
    int beeblebrox{42};
    auto sut = is_invocable<decltype(beeblebrox), void>::value;
    EXPECT_FALSE(sut);
}

TEST(TypeTraitsTest, HasSignatureResolvesToTrue)
{
    auto lambda = [](int foo) -> int { return foo++; };
    auto sut = has_signature<decltype(lambda), int(int)>::value;
    EXPECT_TRUE(sut);
}

TEST(TypeTraitsTest, HasSignatureResolvesToFalse)
{
    auto lambda = [](float foo) -> float { return foo++; };
    auto sut = has_signature<decltype(lambda), void(void)>::value;
    EXPECT_FALSE(sut);
}

TEST(TypeTraitsTest, NotSameIsTrue)
{
    auto sut = not_same<int, float>::value;
    EXPECT_TRUE(sut);
}

TEST(TypeTraitsTest, NotSameIsFalse)
{
    auto sut = not_same<int, int>::value;
    EXPECT_FALSE(sut);
}
