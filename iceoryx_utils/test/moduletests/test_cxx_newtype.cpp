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

template <typename T>
class UniqueID : public cxx::NewType<int, cxx::newtype::Comparable, cxx::newtype::Sortable>
{
  public:
    using type_t = cxx::NewType<int, cxx::newtype::Comparable, cxx::newtype::Sortable>;
    using type_t::type_t;

    UniqueID()
        : type_t(1)
    {
    }

  private:
    explicit UniqueID(const int& a)
        : type_t(a)
    {
    }
};

TEST(NewType, ComparableDoesCompile)
{
    cxx::NewType<int, cxx::newtype::Comparable> a(123), b(345);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a == b);
}

TEST(NewType, SortableDoesCompile)
{
    cxx::NewType<int, cxx::newtype::Sortable> a(456), b(789);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
}

TEST(NewType, ComplexInheritance)
{
    UniqueID<int> a;
}
