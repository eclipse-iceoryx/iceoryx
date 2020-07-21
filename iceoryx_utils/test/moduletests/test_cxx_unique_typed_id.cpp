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

#include "iceoryx_utils/cxx/unique_typed_id.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::cxx;

template <typename T>
class UniqueTypedID_test : public Test
{
  protected:
    using UniqueIDType = T;
};
using Implementations = Types<UniqueTypedID<int>, UniqueTypedID<float>>;
/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(UniqueTypedID_test, Implementations);
#pragma GCC diagnostic pop

TYPED_TEST(UniqueTypedID_test, DefaultConstructorIncrementsID)
{
    typename TestFixture::UniqueIDType a, b;
    EXPECT_THAT(a.getID() + 1, Eq(b.getID()));
}

TYPED_TEST(UniqueTypedID_test, CopyConstructorSetsSameID)
{
    typename TestFixture::UniqueIDType a, b(a);
    EXPECT_THAT(a.getID(), Eq(b.getID()));
}

TYPED_TEST(UniqueTypedID_test, CopyConstructorAssignmentSetsSameID)
{
    typename TestFixture::UniqueIDType a, b;
    a = b;
    EXPECT_THAT(a.getID(), Eq(b.getID()));
}

TYPED_TEST(UniqueTypedID_test, MoveConstructorSetsSameID)
{
    typename TestFixture::UniqueIDType a;
    auto id = a.getID();
    decltype(a) b(std::move(a));
    EXPECT_THAT(b.getID(), Eq(id));
}

TYPED_TEST(UniqueTypedID_test, MoveConstructorInvalidatesOrigin)
{
    typename TestFixture::UniqueIDType a;
    auto id = a.getID();
    decltype(a) b(std::move(a));
    EXPECT_THAT(a.getID(), decltype(a)::INVALID_ID);
}

TYPED_TEST(UniqueTypedID_test, MoveAssignmentSetsSameID)
{
    typename TestFixture::UniqueIDType a, b;
    auto id = a.getID();
    b = std::move(a);
    EXPECT_THAT(b.getID(), Eq(id));
}

TYPED_TEST(UniqueTypedID_test, MoveAssignmentInvalidatesOrigin)
{
    typename TestFixture::UniqueIDType a, b;
    b = std::move(a);
    EXPECT_THAT(a.getID(), Eq(decltype(a)::INVALID_ID));
}

TYPED_TEST(UniqueTypedID_test, SameIDsAreEqual)
{
    typename TestFixture::UniqueIDType a, b(a);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a >= b);
}

TYPED_TEST(UniqueTypedID_test, DifferentIDsAreNotEqual)
{
    typename TestFixture::UniqueIDType a, b;
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a >= b);
}

TYPED_TEST(UniqueTypedID_test, LatestIDIsGreatestID)
{
    typename TestFixture::UniqueIDType a, b;
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
}

TYPED_TEST(UniqueTypedID_test, FirstIDIsSmallestID)
{
    typename TestFixture::UniqueIDType a, b;
    EXPECT_FALSE(b < a);
    EXPECT_FALSE(b <= a);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);
}

TYPED_TEST(UniqueTypedID_test, ConversionToUint64)
{
    typename TestFixture::UniqueIDType a, b;
    uint64_t id = a;
    b = a;
    EXPECT_EQ(id, b);
}

