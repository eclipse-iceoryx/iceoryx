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

#include "test.hpp"

#include "iceoryx_hoofs/data_structures/prefix_tree.hpp"

namespace
{
using namespace ::testing;

using namespace iox;

struct Integer
{
    int x;
};

template <uint32_t Capacity = 8>
using TestPrefixTree = PrefixTree<Integer, Capacity>;

class PrefixTree_test : public ::testing::Test
{
  protected:
    PrefixTree_test()
    {
    }

    ~PrefixTree_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    TestPrefixTree<> sut;
};

TEST_F(PrefixTree_test, ctorConstructsEmptyTree)
{
    EXPECT_TRUE(sut.empty());
}

TEST_F(PrefixTree_test, insertionInEmptyTreeWorks)
{
    auto result = sut.insert("abc", Integer{73});
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(sut.size(), 1U);
}

} // namespace
