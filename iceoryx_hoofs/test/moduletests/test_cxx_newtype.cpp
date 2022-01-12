// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/newtype.hpp"
#include "iceoryx_hoofs/testing/compile_test.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::cxx;

template <typename T>
struct Sut : public T
{
    using T::T;
    using T::operator=; // bring all operator= into scope

    // implement ctors and assignment operators when they are implemented by the base class
    // this is necessary to prevent warnings from some compilers
    Sut() noexcept = default;
    Sut(const Sut&) noexcept = default;
    Sut(Sut&&) noexcept = default;
    Sut& operator=(const Sut&) noexcept = default;
    Sut& operator=(Sut&&) noexcept = default;
};

static CompileTest compileTest(R"(
    #include \"iceoryx_hoofs/cxx/newtype.hpp\"

    template <typename T>
    struct Sut : public T
    {
        using T::T;
        using T::operator=; // bring all operator= into scope
    };

    using namespace iox;
    using namespace iox::cxx;
)",
                               {"iceoryx_hoofs/include"});


TEST(NewType, ComparableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "a2c2823b-3593-4d45-845d-fea249362f11");
    Sut<cxx::NewType<int, newtype::ConstructByValueCopy, newtype::Comparable>> a(123), b(456);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

TEST(NewType, DISABLED_NoComparableDoesNotCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "17ca57e1-8a9c-4235-91bd-1905e580dbb1");
    const char* p = R"(
        Sut<NewType<int, newtype::ConstructByValueCopy>> a(123), b(456);
        if ( a == b ) {}
    )";
    EXPECT_FALSE(compileTest.verify(p));
}

TEST(NewType, SortableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "d58a0838-bad5-4999-b4a5-607b11608f6a");
    Sut<cxx::NewType<int, newtype::ConstructByValueCopy, cxx::newtype::Sortable>> a(456), b(789);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
}

TEST(NewType, DefaultConstructableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "1e6e1d83-36b7-4f9a-9410-438c00a748a9");
    Sut<cxx::NewType<int, newtype::DefaultConstructable>> a;
}

TEST(NewType, CopyConstructableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "177491d2-a940-4584-a362-f973f93b0445");
    Sut<cxx::NewType<int, newtype::ConstructByValueCopy, newtype::CopyConstructable, newtype::Comparable>> a(91), b(92),
        c(a);
    EXPECT_TRUE(a == c);
}

TEST(NewType, CopyAssignableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab690ed0-738e-4e6f-932a-01c9520b5d35");
    Sut<cxx::NewType<int, newtype::ConstructByValueCopy, newtype::CopyAssignable, newtype::Comparable>> a(491), b(492),
        c(423);

    b = a;
    EXPECT_TRUE(a == b);
}

TEST(NewType, MoveConstructableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "635b07e6-0d0d-49b4-ae27-593b870ad45b");
    Sut<cxx::NewType<int, newtype::ConstructByValueCopy, newtype::MoveConstructable, newtype::Comparable>> b(92), c(92),
        d(std::move(c));
    EXPECT_TRUE(b == d);
}

TEST(NewType, MoveAssignableDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "4d8b1166-94d4-4e4c-8759-04984ce3fbec");
    Sut<cxx::NewType<int, newtype::ConstructByValueCopy, newtype::MoveAssignable, newtype::Comparable>> b(912), c(912),
        d(123);
    d = std::move(c);
    EXPECT_TRUE(b == d);
}

TEST(NewType, ConversionDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "6c7cd3e1-1520-43a9-ad45-7269c123b98d");
    Sut<cxx::NewType<int, newtype::ConstructByValueCopy, newtype::Convertable>> a(911);
    int b = static_cast<int>(a);
    EXPECT_THAT(b, Eq(911));
}

TEST(NewType, AssignByValueCopyDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "65a6a726-1324-4b81-b12d-7ca89e149aa2");
    Sut<cxx::NewType<int, newtype::AssignByValueCopy, newtype::ConstructByValueCopy, newtype::Comparable>> a(8791),
        b(651);

    int blubb = 651;
    a = blubb;

    EXPECT_TRUE(a == b);
}

TEST(NewType, AssignByValueMoveDoesCompile)
{
    ::testing::Test::RecordProperty("TEST_ID", "cf62fac7-2d7e-4a70-869b-32a3d29acd10");
    Sut<cxx::NewType<int, newtype::AssignByValueMove, newtype::ConstructByValueCopy, newtype::Comparable>> a(8791),
        b(651);

    int blubb = 651;
    a = std::move(blubb);

    EXPECT_TRUE(a == b);
}
} // namespace
