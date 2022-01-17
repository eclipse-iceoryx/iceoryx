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

#include "iceoryx_posh/roudi/memory/generic_memory_block.hpp"

#include "mocks/roudi_memory_provider_mock.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;

using namespace iox::roudi;

class GenericMemoryBlock_POD_Test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    using PodType = uint32_t;
    GenericMemoryBlock<PodType> sutPOD;
    MemoryProviderTestImpl memoryProvider;
};

TEST_F(GenericMemoryBlock_POD_Test, Initial)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4eb7db4-8e19-450d-bee9-b9a82dbb7e08");
    EXPECT_THAT(sutPOD.value().has_value(), Eq(false));
}

TEST_F(GenericMemoryBlock_POD_Test, Size)
{
    ::testing::Test::RecordProperty("TEST_ID", "2a661e69-cf6f-4578-8348-86454a350856");
    EXPECT_THAT(sutPOD.size(), Eq(sizeof(PodType)));
}

TEST_F(GenericMemoryBlock_POD_Test, Alignment)
{
    ::testing::Test::RecordProperty("TEST_ID", "7acbafcd-1c4c-4c6a-976c-fc3ac8035cd3");
    EXPECT_THAT(sutPOD.size(), Eq(alignof(PodType)));
}

TEST_F(GenericMemoryBlock_POD_Test, EmplaceWithoutCreate)
{
    ::testing::Test::RecordProperty("TEST_ID", "a6ded0ad-72e2-4c0a-b9ae-b359e0044350");
    constexpr uint32_t EXPECTED_VALUE{37};
    EXPECT_THAT(sutPOD.emplace(EXPECTED_VALUE).has_value(), Eq(false));
}

TEST_F(GenericMemoryBlock_POD_Test, EmplaceValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "f9527cb7-33a9-40fe-b573-39c2b02033f9");
    constexpr uint32_t EXPECTED_VALUE{42};
    IOX_DISCARD_RESULT(memoryProvider.addMemoryBlock(&sutPOD));
    IOX_DISCARD_RESULT(memoryProvider.create());
    ASSERT_THAT(sutPOD.memory().has_value(), Eq(true));

    auto emplaceResult = sutPOD.emplace(EXPECTED_VALUE);
    ASSERT_THAT(emplaceResult.has_value(), Eq(true));
    EXPECT_THAT(*emplaceResult.value(), Eq(EXPECTED_VALUE));
}

TEST_F(GenericMemoryBlock_POD_Test, MultipleEmplaceValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "cb0175fa-1f9f-4fc8-802f-4eee14fa9ad9");
    constexpr uint32_t FIRST_VALUE{13};
    constexpr uint32_t EXPECTED_VALUE{73};
    IOX_DISCARD_RESULT(memoryProvider.addMemoryBlock(&sutPOD));
    IOX_DISCARD_RESULT(memoryProvider.create());

    sutPOD.emplace(FIRST_VALUE);

    auto emplaceResult = sutPOD.emplace(EXPECTED_VALUE);
    ASSERT_THAT(emplaceResult.has_value(), Eq(true));
    EXPECT_THAT(*emplaceResult.value(), Eq(EXPECTED_VALUE));
}

TEST_F(GenericMemoryBlock_POD_Test, GetValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "01db1013-4df4-4e24-b1a6-9406153ac693");
    constexpr uint32_t EXPECTED_VALUE{42};
    IOX_DISCARD_RESULT(memoryProvider.addMemoryBlock(&sutPOD));
    IOX_DISCARD_RESULT(memoryProvider.create());

    sutPOD.emplace(EXPECTED_VALUE);

    ASSERT_THAT(sutPOD.value().has_value(), Eq(true));
    EXPECT_THAT(*(*sutPOD.value()), Eq(EXPECTED_VALUE));
}

class NonTrivialClass
{
  public:
    NonTrivialClass(uint32_t data)
        : m_data(data)
    {
        ++s_constructorCounter;
    }

    ~NonTrivialClass()
    {
        ++s_destructorCounter;
    }

    uint32_t m_data{0};

    static uint64_t s_constructorCounter;
    static uint64_t s_destructorCounter;

    static void resetCounter()
    {
        s_constructorCounter = 0;
        s_destructorCounter = 0;
    }
};

uint64_t NonTrivialClass::s_constructorCounter = 0;
uint64_t NonTrivialClass::s_destructorCounter = 0;

class GenericMemoryBlock_NonTrivial_Test : public Test
{
  public:
    void SetUp() override
    {
        NonTrivialClass::resetCounter();
        EXPECT_THAT(NonTrivialClass::s_constructorCounter, Eq(0u));
        EXPECT_THAT(NonTrivialClass::s_destructorCounter, Eq(0u));
    }

    void TearDown() override
    {
    }

    GenericMemoryBlock<NonTrivialClass> sut;
    MemoryProviderTestImpl memoryProvider;
};

TEST_F(GenericMemoryBlock_NonTrivial_Test, EmplaceValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "6758f87f-dd40-4d96-a593-3c8c5ad754a1");
    constexpr uint32_t EXPECTED_VALUE{142};
    IOX_DISCARD_RESULT(memoryProvider.addMemoryBlock(&sut));
    IOX_DISCARD_RESULT(memoryProvider.create());
    ASSERT_THAT(sut.memory().has_value(), Eq(true));

    auto emplaceResult = sut.emplace(EXPECTED_VALUE);
    EXPECT_THAT(NonTrivialClass::s_constructorCounter, Eq(1u));
    ASSERT_THAT(emplaceResult.has_value(), Eq(true));
    EXPECT_THAT(emplaceResult.value()->m_data, Eq(EXPECTED_VALUE));
}

TEST_F(GenericMemoryBlock_NonTrivial_Test, MultipleEmplaceValue)
{
    ::testing::Test::RecordProperty("TEST_ID", "4894ac48-6f29-4f9f-b56d-ed559432d2e3");
    constexpr uint32_t FIRST_VALUE{113};
    constexpr uint32_t EXPECTED_VALUE{173};
    IOX_DISCARD_RESULT(memoryProvider.addMemoryBlock(&sut));
    IOX_DISCARD_RESULT(memoryProvider.create());

    sut.emplace(FIRST_VALUE);

    auto emplaceResult = sut.emplace(EXPECTED_VALUE);
    EXPECT_THAT(NonTrivialClass::s_constructorCounter, Eq(2u));
    ASSERT_THAT(emplaceResult.has_value(), Eq(true));
    EXPECT_THAT(emplaceResult.value()->m_data, Eq(EXPECTED_VALUE));
}

TEST_F(GenericMemoryBlock_NonTrivial_Test, RunDestructorWithoutCreate)
{
    ::testing::Test::RecordProperty("TEST_ID", "58f06511-2ee9-43cd-903a-d621674328de");
    /// @note we just expect to not terminate
}

TEST_F(GenericMemoryBlock_NonTrivial_Test, RunDestructorWithoutEmplace)
{
    ::testing::Test::RecordProperty("TEST_ID", "f4861eae-7808-4a48-b9f1-46159cad301c");
    IOX_DISCARD_RESULT(memoryProvider.addMemoryBlock(&sut));
    IOX_DISCARD_RESULT(memoryProvider.create());
    /// @note we just expect to not terminate
}

TEST_F(GenericMemoryBlock_NonTrivial_Test, DestroyWithEmplace)
{
    ::testing::Test::RecordProperty("TEST_ID", "e5a1568a-3b46-4c1c-96f0-c191de1b19b7");
    constexpr uint32_t EXPECTED_VALUE{111};
    IOX_DISCARD_RESULT(memoryProvider.addMemoryBlock(&sut));
    IOX_DISCARD_RESULT(memoryProvider.create());
    EXPECT_THAT(sut.emplace(EXPECTED_VALUE).value()->m_data, EXPECTED_VALUE);
    EXPECT_THAT(NonTrivialClass::s_constructorCounter, Eq(1u));

    IOX_DISCARD_RESULT(memoryProvider.destroy());

    EXPECT_THAT(sut.value().has_value(), Eq(false));
    EXPECT_THAT(NonTrivialClass::s_destructorCounter, Eq(1u));
}

TEST_F(GenericMemoryBlock_NonTrivial_Test, RepetitiveDestroyWithEmplace)
{
    ::testing::Test::RecordProperty("TEST_ID", "e8e07e99-7896-4cdc-b990-b24522d7d504");
    constexpr uint32_t EXPECTED_VALUE{42};
    IOX_DISCARD_RESULT(memoryProvider.addMemoryBlock(&sut));
    IOX_DISCARD_RESULT(memoryProvider.create());
    sut.emplace(EXPECTED_VALUE);

    IOX_DISCARD_RESULT(memoryProvider.destroy());

    EXPECT_THAT(sut.value().has_value(), Eq(false));

    IOX_DISCARD_RESULT(memoryProvider.destroy());
    IOX_DISCARD_RESULT(memoryProvider.destroy());

    EXPECT_THAT(NonTrivialClass::s_destructorCounter, Eq(1u));
}

} // namespace
