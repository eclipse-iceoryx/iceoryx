// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/roudi/introspection/fixed_size_container.hpp"
#include "test.hpp"

#include <cstdint>

namespace
{
using namespace ::testing;

using namespace iox::roudi;

class FixedSizeContainer_test : public Test
{
  public:
    using index_t = int32_t;
    int32_t NOT_AN_ELEMENT = -1;
    FixedSizeContainer_test()
    {
    }
    ~FixedSizeContainer_test()
    {
    }

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(FixedSizeContainer_test, addSingleElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "07b8892c-947e-40a8-aaed-a24dacdc2850");
    FixedSizeContainer<int, 1> container;
    EXPECT_THAT(container.add(12), Ne(NOT_AN_ELEMENT));
    EXPECT_THAT(container.add(12), Eq(NOT_AN_ELEMENT));
}

TEST_F(FixedSizeContainer_test, addMultiElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "78024408-52e1-41ea-8b0a-7cfefe516cb1");
    constexpr uint32_t capacity = 123;
    FixedSizeContainer<int, capacity> container;
    for (uint32_t k = 0; k < capacity; ++k)
    {
        EXPECT_THAT(container.add(12), Ne(NOT_AN_ELEMENT));
    }

    for (uint32_t k = 0; k < capacity; ++k)
    {
        EXPECT_THAT(container.add(12), Eq(NOT_AN_ELEMENT));
    }
}

TEST_F(FixedSizeContainer_test, removeAndSizeSingleElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "614bd693-7427-4557-90a3-5dbcf6ea7ff6");
    FixedSizeContainer<int, 1> container;
    EXPECT_THAT(container.size(), Eq(0u));
    container.remove(0);
    EXPECT_THAT(container.size(), Eq(0u));

    EXPECT_THAT(container.add(123), Ne(NOT_AN_ELEMENT));
    EXPECT_THAT(container.size(), Eq(1u));
    EXPECT_THAT(container.add(123), Eq(NOT_AN_ELEMENT));
    EXPECT_THAT(container.size(), Eq(1u));
    EXPECT_THAT(container.add(123), Eq(NOT_AN_ELEMENT));
    EXPECT_THAT(container.size(), Eq(1u));

    container.remove(0);
    EXPECT_THAT(container.size(), Eq(0u));
    container.remove(0);
    EXPECT_THAT(container.size(), Eq(0u));

    EXPECT_THAT(container.add(123), Ne(NOT_AN_ELEMENT));
    EXPECT_THAT(container.size(), Eq(1u));
}

TEST_F(FixedSizeContainer_test, removeAndSizeMultiElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "20a93c9c-7c80-494a-a85d-6313efe2f99e");
    constexpr uint32_t capacity = 100;
    FixedSizeContainer<int, capacity> container;

    for (uint32_t k = 0; k < capacity; ++k)
    {
        EXPECT_THAT(container.add(12), Ne(NOT_AN_ELEMENT));
        EXPECT_THAT(container.size(), Eq(k + 1));
    }

    for (uint32_t k = 0; k < capacity; ++k)
    {
        container.remove(static_cast<index_t>(k));
        EXPECT_THAT(container.size(), Eq(capacity - k - 1));
    }

    for (uint32_t k = 0; k < capacity; ++k)
    {
        container.add(12);
        container.add(12);
        container.remove(static_cast<index_t>(k));
        container.add(12);

        size_t newSize = 2 * (k + 1);
        if (newSize > 100)
        {
            newSize = 100;
        }

        EXPECT_THAT(container.size(), Eq(newSize));
    }

    for (uint32_t k = 0; k < capacity; ++k)
    {
        container.remove(static_cast<index_t>(k));
        EXPECT_THAT(container.size(), Eq(capacity - k - 1));
    }
}

TEST_F(FixedSizeContainer_test, addAndVerifySingleElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "c0dabbf3-6d68-4596-9819-1de011fbe272");
    FixedSizeContainer<int, 1> container;

    EXPECT_THAT(container.get(0), Eq(nullptr));
    container.add(1337);
    EXPECT_THAT(*container.get(0), Eq(1337));
    EXPECT_THAT(container[0], Eq(1337));
    container.add(42);
    EXPECT_THAT(*container.get(0), Eq(1337));
}

TEST_F(FixedSizeContainer_test, addAndVerifyMultiElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "de654427-1873-4583-b6a6-117869ca86f0");
    constexpr uint32_t capacity = 25;
    FixedSizeContainer<size_t, capacity> container;

    for (uint32_t i = 0; i < capacity; ++i)
    {
        for (uint32_t k = i; k < capacity; ++k)
        {
            EXPECT_THAT(container.get(static_cast<index_t>(k)), Eq(nullptr));
        }

        container.add(2 * i + 1);

        for (uint32_t k = 0; k < i; ++k)
        {
            EXPECT_THAT(*container.get(static_cast<index_t>(k)), Eq(2 * k + 1));
            EXPECT_THAT(container[static_cast<index_t>(k)], Eq(2 * k + 1));
        }
    }
}

TEST_F(FixedSizeContainer_test, removeAndVerifySingleElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "221a8117-673c-445e-b1fa-88fe2117d440");
    FixedSizeContainer<int, 1> container;

    EXPECT_THAT(container.get(0), Eq(nullptr));
    container.add(1337);
    EXPECT_THAT(*container.get(0), Eq(1337));
    container.remove(0);
    EXPECT_THAT(container.get(0), Eq(nullptr));
}

TEST_F(FixedSizeContainer_test, removeAndVerifyMultiElementContainer)
{
    ::testing::Test::RecordProperty("TEST_ID", "ad112c45-1166-41f9-8cd0-af604677957f");
    constexpr uint32_t capacity = 25;
    FixedSizeContainer<size_t, capacity> container;
    for (uint32_t i = 0; i < capacity; ++i)
    {
        container.add(5 * i + 12);
    }

    for (uint32_t i = 0; i < capacity; ++i)
    {
        for (uint32_t k = 0; k < i; ++k)
        {
            EXPECT_THAT(container.get(static_cast<index_t>(k)), Eq(nullptr));
        }
        container.remove(static_cast<index_t>(i));
        for (uint32_t k = i + 1; k < capacity; ++k)
        {
            EXPECT_THAT(*container.get(static_cast<index_t>(k)), Eq(5 * k + 12));
        }
    }
}

} // namespace
