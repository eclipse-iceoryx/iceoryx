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

#include "iceoryx_utils/cxx/newtype.hpp"
#include "test.hpp"

using namespace iox;
using namespace iox::cxx;

TEST(NewType, ComparableDoesCompile)
{
    cxx::NewType<int, newtype::ConstructByValueCopy, newtype::Comparable> a(123), b(456);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

TEST(NewType, SortableDoesCompile)
{
    cxx::NewType<int, newtype::ConstructByValueCopy, cxx::newtype::Sortable> a(456), b(789);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
}

TEST(NewType, CopyableDoesCompile)
{
    cxx::NewType<int, newtype::ConstructByValueCopy, newtype::Copyable, newtype::Comparable> a(91), b(92), c(a);
    a = b;
    EXPECT_TRUE(a == b);
}

TEST(NewType, MovableDoesCompile)
{
    cxx::NewType<int, newtype::ConstructByValueCopy, newtype::Movable, newtype::Comparable> a(91), b(92), c(92),
        d(std::move(c));
    a = std::move(b);
    EXPECT_TRUE(a == d);
}
