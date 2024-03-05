// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2023 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#include "iox/newtype.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;

struct ComplexType
{
    uint64_t value{0};

    // the test requires the type to be implicit convertable
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    ComplexType(uint64_t v)
        : value(v)
    {
    }

    ComplexType(const ComplexType& other) noexcept = default;
    ComplexType(ComplexType&& other) noexcept
        : value{other.value}
    {
        other.value = 0;
    }

    ComplexType& operator=(const ComplexType& rhs) noexcept = default;

    ComplexType& operator=(ComplexType&& rhs) noexcept
    {
        if (this != &rhs)
        {
            value = rhs.value;
            rhs.value = 0;
        }
        return *this;
    }

    bool operator==(const ComplexType& rhs) const
    {
        return value == rhs.value;
    }
};

TEST(NewType, ComparableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2c2823b-3593-4d45-845d-fea249362f11");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::Comparable);

    SutType a(123);
    SutType b(456);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

TEST(NewType, SortableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "d58a0838-bad5-4999-b4a5-607b11608f6a");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::Sortable);

    SutType a(456);
    SutType b(789);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
}

TEST(NewType, DefaultConstructableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e6e1d83-36b7-4f9a-9410-438c00a748a9");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::DefaultConstructable);

    SutType a;
}

TEST(NewType, CopyConstructableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "177491d2-a940-4584-a362-f973f93b0445");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::CopyConstructable, newtype::Comparable);

    SutType a(91);
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization) copy constructor shall be tested
    SutType c(a);
    EXPECT_TRUE(a == c);
}

TEST(NewType, CopyConstructableComplexTypeDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "c73499b8-c8b0-4cc1-b097-44a18f571d34");
    IOX_NEW_TYPE(SutType, ComplexType, newtype::ConstructByValueCopy, newtype::CopyConstructable, newtype::Comparable);

    SutType a(91);
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization) copy constructor shall be tested
    SutType c(a);
    EXPECT_TRUE(a == c);
}

TEST(NewType, CopyAssignableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab690ed0-738e-4e6f-932a-01c9520b5d35");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::CopyAssignable, newtype::Comparable);

    SutType a(491);
    SutType b(492);
    SutType c(491);

    b = a;
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(b == c);
}

TEST(NewType, CopyAssignableComplexTypeDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "011efe73-7700-41c1-bc12-8aa4e848b0ce");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::CopyAssignable, newtype::Comparable);

    SutType a(491);
    SutType b(492);
    SutType c(491);

    b = a;
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(b == c);
}

TEST(NewType, MoveConstructableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "635b07e6-0d0d-49b4-ae27-593b870ad45b");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::MoveConstructable, newtype::Comparable);

    SutType b(92);
    SutType c(92);
    SutType d(std::move(c));
    EXPECT_TRUE(b == d);
}

TEST(NewType, MoveConstructableComplexTypeDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "7bba277d-5704-4ff7-810d-74bbb851469a");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::MoveConstructable, newtype::Comparable);

    SutType b(92);
    SutType c(92);
    SutType d(std::move(c));
    EXPECT_TRUE(b == d);
}

TEST(NewType, MoveAssignableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "4d8b1166-94d4-4e4c-8759-04984ce3fbec");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::MoveAssignable, newtype::Comparable);

    SutType b(912);
    SutType c(912);
    SutType d(123);
    d = std::move(c);
    EXPECT_TRUE(b == d);
}

TEST(NewType, MoveAssignableComplexTypeDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "c300724e-c7ae-4897-ac99-62b0c4f44fbe");
    IOX_NEW_TYPE(SutType, ComplexType, newtype::ConstructByValueCopy, newtype::MoveAssignable, newtype::Comparable);

    SutType b(912);
    SutType c(912);
    SutType d(123);
    d = std::move(c);
    EXPECT_TRUE(b == d);
}

TEST(NewType, ConversionDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c7cd3e1-1520-43a9-ad45-7269c123b98d");
    IOX_NEW_TYPE(SutType, int, newtype::ConstructByValueCopy, newtype::Convertable);
    SutType a(911);
    int b = static_cast<int>(a);
    EXPECT_THAT(b, Eq(911));
}

TEST(NewType, AssignByValueCopyDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "65a6a726-1324-4b81-b12d-7ca89e149aa2");
    IOX_NEW_TYPE(SutType, int, newtype::AssignByValueCopy, newtype::ConstructByValueCopy, newtype::Comparable);

    SutType a(8791);
    SutType b(651);

    int blubb = 651;
    a = blubb;

    EXPECT_TRUE(a == b);
}

TEST(NewType, AssignByValueCopyComplexTypeDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "9c341f63-4409-452a-bbe4-d05a42b9bd91");
    IOX_NEW_TYPE(SutType, ComplexType, newtype::AssignByValueCopy, newtype::ConstructByValueCopy, newtype::Comparable);

    SutType a(8791);
    SutType b(651);

    ComplexType blubb = 651;
    a = blubb;

    EXPECT_TRUE(a == b);
}

TEST(NewType, AssignByValueMoveDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf62fac7-2d7e-4a70-869b-32a3d29acd10");
    IOX_NEW_TYPE(SutType, int, newtype::AssignByValueMove, newtype::ConstructByValueCopy, newtype::Comparable);

    SutType a(8791);
    SutType b(651);

    int blubb = 651;
    // NOLINTNEXTLINE(hicpp-move-const-arg,performance-move-const-arg) move assignment shall be tested
    a = std::move(blubb);

    EXPECT_TRUE(a == b);
}

TEST(NewType, AssignByValueMoveComplexTypeDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "dc23e4e2-833b-4cd9-80a1-28f627544836");
    IOX_NEW_TYPE(SutType, ComplexType, newtype::AssignByValueMove, newtype::ConstructByValueCopy, newtype::Comparable);

    SutType a(8791);
    SutType b(651);

    ComplexType blubb = 651;
    a = std::move(blubb);

    EXPECT_TRUE(a == b);
}

TEST(NewType, CreatingNewTypeWithMacroWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d43d41f6-c6d8-4523-a7cf-8f86822643cc");
    IOX_NEW_TYPE(SutType, uint64_t, newtype::ConstructByValueCopy, newtype::Comparable);

    SutType a(73);
    SutType b(37);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

TEST(NewType, NewTypeIsPreIncrementable)
{
    ::testing::Test::RecordProperty("TEST_ID", "6d03b24b-fc72-409b-aa2a-f19228ff152c");
    IOX_NEW_TYPE(SutType,
                 uint64_t,
                 newtype::MoveConstructable,
                 newtype::CopyConstructable,
                 newtype::Comparable,
                 newtype::Incrementable,
                 newtype::ConstructByValueCopy);

    constexpr uint64_t START_VALUE{42};
    SutType a{START_VALUE};
    SutType pre{++a};
    EXPECT_THAT(pre, Eq(SutType{START_VALUE + 1}));
    EXPECT_THAT(a, Eq(SutType{START_VALUE + 1}));
}

TEST(NewType, NewTypeIsPostIncrementable)
{
    ::testing::Test::RecordProperty("TEST_ID", "c6b273bc-ef1a-43f0-b98f-d247b19e50f3");
    IOX_NEW_TYPE(SutType,
                 uint64_t,
                 newtype::MoveConstructable,
                 newtype::CopyConstructable,
                 newtype::Comparable,
                 newtype::Incrementable,
                 newtype::ConstructByValueCopy);

    constexpr uint64_t START_VALUE{42};
    SutType a{START_VALUE};
    SutType post{a++};
    EXPECT_THAT(post, Eq(SutType{START_VALUE}));
    EXPECT_THAT(a, Eq(SutType{START_VALUE + 1}));
}

TEST(NewType, NewTypeIsPreDecrementable)
{
    ::testing::Test::RecordProperty("TEST_ID", "27262c86-2509-4c55-8bff-a37337e79b67");
    IOX_NEW_TYPE(SutType,
                 uint64_t,
                 newtype::MoveConstructable,
                 newtype::CopyConstructable,
                 newtype::Comparable,
                 newtype::Decrementable,
                 newtype::ConstructByValueCopy);

    constexpr uint64_t START_VALUE{24};
    SutType a{START_VALUE};
    SutType pre{--a};
    EXPECT_THAT(pre, Eq(SutType{START_VALUE - 1}));
    EXPECT_THAT(a, Eq(SutType{START_VALUE - 1}));
}

TEST(NewType, NewTypeIsPostDecrementable)
{
    ::testing::Test::RecordProperty("TEST_ID", "48e52551-6c7e-441d-a755-9f233607b5c8");
    IOX_NEW_TYPE(SutType,
                 uint64_t,
                 newtype::MoveConstructable,
                 newtype::CopyConstructable,
                 newtype::Comparable,
                 newtype::Decrementable,
                 newtype::ConstructByValueCopy);

    constexpr uint64_t START_VALUE{24};
    SutType a{START_VALUE};
    SutType post{a--};
    EXPECT_THAT(post, Eq(SutType{START_VALUE}));
    EXPECT_THAT(a, Eq(SutType{START_VALUE - 1}));
}

TEST(NewType, NewTypeCanBeAdded)
{
    ::testing::Test::RecordProperty("TEST_ID", "a36d1031-e7b0-4931-bd08-a67ffa367ffe");
    IOX_NEW_TYPE(SutType,
                 uint64_t,
                 newtype::MoveConstructable,
                 newtype::CopyConstructable,
                 newtype::Comparable,
                 newtype::Arithmetic,
                 newtype::ConstructByValueCopy);

    constexpr uint64_t START_VALUE{42};
    SutType a{START_VALUE};
    SutType b{START_VALUE};
    SutType c{a + b};
    EXPECT_EQ(c, SutType{START_VALUE + START_VALUE});
}

TEST(NewType, NewTypeCanBeSubstracted)
{
    ::testing::Test::RecordProperty("TEST_ID", "3bc5fdc7-33b1-4556-b4aa-c6c18ccb9e1d");
    IOX_NEW_TYPE(SutType,
                 uint64_t,
                 newtype::MoveConstructable,
                 newtype::CopyConstructable,
                 newtype::Comparable,
                 newtype::Arithmetic,
                 newtype::ConstructByValueCopy);

    constexpr uint64_t START_VALUE{42};
    SutType a{START_VALUE};
    SutType b{START_VALUE};
    SutType c{a - b};
    EXPECT_THAT(c, Eq(SutType{START_VALUE - START_VALUE}));
}

TEST(NewType, NewTypeCanBeMultiplied)
{
    ::testing::Test::RecordProperty("TEST_ID", "e51f9818-8a52-466a-92ef-bbd7258d96b8");
    IOX_NEW_TYPE(SutType,
                 uint64_t,
                 newtype::MoveConstructable,
                 newtype::CopyConstructable,
                 newtype::Comparable,
                 newtype::Arithmetic,
                 newtype::ConstructByValueCopy);

    constexpr uint64_t START_VALUE1{42};
    constexpr uint64_t START_VALUE2{24};
    SutType a{START_VALUE1};
    SutType b{START_VALUE2};
    SutType c{a * b};
    EXPECT_THAT(c, Eq(SutType{START_VALUE1 * START_VALUE2}));
}

TEST(NewType, NewTypeCanBeDivided)
{
    ::testing::Test::RecordProperty("TEST_ID", "95c15cc9-7fb2-4deb-838b-f387c718ece8");
    IOX_NEW_TYPE(SutType,
                 uint64_t,
                 newtype::MoveConstructable,
                 newtype::CopyConstructable,
                 newtype::Comparable,
                 newtype::Arithmetic,
                 newtype::ConstructByValueCopy);

    constexpr uint64_t START_VALUE1{42};
    constexpr uint64_t START_VALUE2{24};
    SutType a{START_VALUE1};
    SutType b{START_VALUE2};
    SutType c{a / b};
    EXPECT_THAT(c, Eq(SutType{START_VALUE1 / START_VALUE2}));
}

} // namespace
