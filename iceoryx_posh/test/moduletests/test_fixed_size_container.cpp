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

#define private public
#define protected public

#include "iceoryx_posh/internal/roudi/introspection/fixed_size_container.hpp"

#undef private
#undef protected

#include <cstdint>

using namespace ::testing;
using ::testing::Return;

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

    virtual void SetUp()
    {
        internal::CaptureStdout();
    }

    virtual void TearDown()
    {
        std::string output = internal::GetCapturedStdout();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};

TEST_F(FixedSizeContainer_test, addSingleElementContainer)
{
    FixedSizeContainer<int, 1> container;
    EXPECT_THAT(container.add(12), Ne(NOT_AN_ELEMENT));
    EXPECT_THAT(container.add(12), Eq(NOT_AN_ELEMENT));
}

TEST_F(FixedSizeContainer_test, addMultiElementContainer)
{
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
    constexpr uint32_t capacity = 100;
    FixedSizeContainer<int, capacity> container;

    for (uint32_t k = 0; k < capacity; ++k)
    {
        EXPECT_THAT(container.add(12), Ne(NOT_AN_ELEMENT));
        EXPECT_THAT(container.size(), Eq(k + 1));
    }

    for (uint32_t k = 0; k < capacity; ++k)
    {
        container.remove(k);
        EXPECT_THAT(container.size(), Eq(capacity - k - 1));
    }

    for (uint32_t k = 0; k < capacity; ++k)
    {
        container.add(12);
        container.add(12);
        container.remove(k);
        container.add(12);

        size_t newSize = 2 * (k + 1);
        if (newSize > 100)
            newSize = 100;

        EXPECT_THAT(container.size(), Eq(newSize));
    }

    for (uint32_t k = 0; k < capacity; ++k)
    {
        container.remove(k);
        EXPECT_THAT(container.size(), Eq(capacity - k - 1));
    }
}

TEST_F(FixedSizeContainer_test, addAndVerifySingleElementContainer)
{
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
    constexpr size_t capacity = 25;
    FixedSizeContainer<size_t, capacity> container;

    for (size_t i = 0; i < capacity; ++i)
    {
        for (size_t k = i; k < capacity; ++k)
        {
            EXPECT_THAT(container.get(k), Eq(nullptr));
        }

        container.add(2 * i + 1);

        for (size_t k = 0; k < i; ++k)
        {
            EXPECT_THAT(*container.get(k), Eq(2 * k + 1));
            EXPECT_THAT(container[k], Eq(2 * k + 1));
        }
    }
}

TEST_F(FixedSizeContainer_test, removeAndVerifySingleElementContainer)
{
    FixedSizeContainer<int, 1> container;

    EXPECT_THAT(container.get(0), Eq(nullptr));
    container.add(1337);
    EXPECT_THAT(*container.get(0), Eq(1337));
    container.remove(0);
    EXPECT_THAT(container.get(0), Eq(nullptr));
}

TEST_F(FixedSizeContainer_test, removeAndVerifyMultiElementContainer)
{
    constexpr size_t capacity = 25;
    FixedSizeContainer<size_t, capacity> container;
    for (size_t i = 0; i < capacity; ++i)
    {
        container.add(5 * i + 12);
    }

    for (size_t i = 0; i < capacity; ++i)
    {
        for (size_t k = 0; k < i; ++k)
        {
            EXPECT_THAT(container.get(k), Eq(nullptr));
        }
        container.remove(i);
        for (size_t k = i + 1; k < capacity; ++k)
        {
            EXPECT_THAT(*container.get(k), Eq(5 * k + 12));
        }
    }
}
