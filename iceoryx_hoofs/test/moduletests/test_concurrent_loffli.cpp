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

#include "iceoryx_hoofs/internal/concurrent/loffli.hpp"
#include "test.hpp"

#include <random>

namespace
{
using namespace ::testing;

constexpr uint32_t Size{4};
using LoFFLiTestSubjects = Types<iox::concurrent::LoFFLi>;

TYPED_TEST_SUITE(LoFFLi_test, LoFFLiTestSubjects, );

template <typename LoFFLiType>
class LoFFLi_test : public Test
{
  public:
    void SetUp() override
    {
        m_loffli.init(&m_memoryLoFFLi[0], Size);
    }

    void TearDown() override
    {
    }

    using LoFFLiIndex_t = typename LoFFLiType::Index_t;
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) needed for LoFFLi::init
    LoFFLiIndex_t m_memoryLoFFLi[LoFFLiType::requiredIndexMemorySize(Size)]{0};
    LoFFLiType m_loffli;
};

TYPED_TEST(LoFFLi_test, Misuse_NullptrMemory)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab877f29-cab0-48ae-a2c0-054633b6415a");
    decltype(this->m_loffli) loFFLi;
    // todo #1196 remove EXPECT_DEATH
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-avoid-goto, cert-err33-c)
    EXPECT_DEATH(loFFLi.init(nullptr, 1), ".*");
}

TYPED_TEST(LoFFLi_test, Misuse_ZeroSize)
{
    ::testing::Test::RecordProperty("TEST_ID", "fb9c797b-22b4-4572-a7a2-eaf13574dbac");
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) needed to test LoFFLi::init
    uint32_t memoryLoFFLi[4];
    decltype(this->m_loffli) loFFLi;
    // todo #1196 remove EXPECT_DEATH
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-avoid-goto, cert-err33-c)
    EXPECT_DEATH(loFFLi.init(&memoryLoFFLi[0], 0), ".*");
}

TYPED_TEST(LoFFLi_test, Misuse_SizeToLarge)
{
    ::testing::Test::RecordProperty("TEST_ID", "14b4b82c-ae2b-4bd2-97cf-93fcea87f050");
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) needed to test LoFFLi::init
    uint32_t memoryLoFFLi[4];
    decltype(this->m_loffli) loFFLi;
    // todo #1196 remove EXPECT_DEATH
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-avoid-goto, cert-err33-c)
    EXPECT_DEATH(loFFLi.init(&memoryLoFFLi[0], UINT32_MAX - 1), ".*");
}


TYPED_TEST(LoFFLi_test, Initialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "d4ee0d3a-313d-4511-a759-f1bb7482c50b");
    // loffli must be full when initialized
    EXPECT_THAT(this->m_loffli.push(0), Eq(false));
}

TYPED_TEST(LoFFLi_test, SinglePop)
{
    ::testing::Test::RecordProperty("TEST_ID", "5ed7c05a-3cee-4895-825e-b39fa127fb97");
    constexpr uint32_t AFFE = 0xAFFE;
    uint32_t index = AFFE;
    EXPECT_THAT(this->m_loffli.pop(index), Eq(true));
    EXPECT_THAT(index, Ne(AFFE));
}

TYPED_TEST(LoFFLi_test, PopEmpty)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f5c9890-14d6-4b63-ae0e-b5754d2e0f23");
    constexpr uint32_t AFFE = 0xAFFE;
    for (uint32_t i = 0; i < Size; i++)
    {
        uint32_t index = AFFE;
        EXPECT_THAT(this->m_loffli.pop(index), Eq(true));
        EXPECT_THAT(index, Eq(i));
    }

    uint32_t index = AFFE;
    EXPECT_THAT(this->m_loffli.pop(index), Eq(false));
    EXPECT_THAT(index, Eq(AFFE));
}

TYPED_TEST(LoFFLi_test, PopFromUninitializedLoFFLi)
{
    ::testing::Test::RecordProperty("TEST_ID", "1f516a22-07ac-43b5-9230-e09e4c202165");
    constexpr uint32_t AFFE = 0xAFFE;
    uint32_t index = AFFE;

    decltype(this->m_loffli) loFFLi;
    EXPECT_THAT(loFFLi.pop(index), Eq(false));
}

TYPED_TEST(LoFFLi_test, SinglePush)
{
    ::testing::Test::RecordProperty("TEST_ID", "0b7bf346-056b-4b1c-a6e9-92b54233598e");
    constexpr uint32_t AFFE = 0xAFFE;
    uint32_t index{0};
    this->m_loffli.pop(index);

    uint32_t indexPush = index;
    EXPECT_THAT(this->m_loffli.push(indexPush), Eq(true));
    index = AFFE;
    EXPECT_THAT(this->m_loffli.pop(index), Eq(true));
    EXPECT_THAT(index, Eq(indexPush));
}

TYPED_TEST(LoFFLi_test, PushTillFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "610e3e8f-1db9-4a92-a5e8-2d1bb0077123");
    std::vector<uint32_t> useList;
    uint32_t index{0};
    while (this->m_loffli.pop(index))
    {
        useList.push_back(index);
    }

    for (const auto& item : useList)
    {
        EXPECT_THAT(this->m_loffli.push(item), Eq(true));
    }
}

TYPED_TEST(LoFFLi_test, PushRandomOrder)
{
    ::testing::Test::RecordProperty("TEST_ID", "73700ca3-3a37-48af-8485-91e0a7b4e3d0");
    std::vector<uint32_t> useListToPush;
    std::vector<uint32_t> useListPoped;
    uint32_t index{0};
    while (this->m_loffli.pop(index))
    {
        useListToPush.push_back(index);
    }

    std::random_device randomDevice;
    std::default_random_engine randomEngine(randomDevice());

    std::shuffle(useListToPush.begin(), useListToPush.end(), randomEngine);

    for (const auto& item : useListToPush)
    {
        EXPECT_THAT(this->m_loffli.push(item), Eq(true));
    }

    while (this->m_loffli.pop(index))
    {
        useListPoped.push_back(index);
    }

    std::sort(useListToPush.begin(), useListToPush.end());
    std::sort(useListPoped.begin(), useListPoped.end());

    EXPECT_THAT(useListPoped, Eq(useListToPush));
}

TYPED_TEST(LoFFLi_test, PushWrongIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "9a3fabd1-17c0-4e40-8390-53ca4d656d91");
    uint32_t index{0};
    this->m_loffli.pop(index);

    uint32_t indexPush = index + 1;
    EXPECT_THAT(this->m_loffli.push(indexPush), Eq(false));
}

TYPED_TEST(LoFFLi_test, PushOutOfBoundIndex)
{
    ::testing::Test::RecordProperty("TEST_ID", "261ba489-1cec-44db-8c41-d9e71c761d91");
    uint32_t index{0};
    this->m_loffli.pop(index);

    EXPECT_THAT(this->m_loffli.push(Size), Eq(false));
    EXPECT_THAT(this->m_loffli.push(Size + 42), Eq(false));
}

TYPED_TEST(LoFFLi_test, PushWhenFull)
{
    ::testing::Test::RecordProperty("TEST_ID", "05505968-a3f9-4a8a-87e3-d32064ecdb66");
    uint32_t indexPush = 0;
    EXPECT_THAT(this->m_loffli.push(indexPush), Eq(false));
}

TYPED_TEST(LoFFLi_test, PushToUninitializedLoFFLi)
{
    ::testing::Test::RecordProperty("TEST_ID", "34f5b48a-a30a-4dd1-81d6-7c963c005f1b");
    decltype(this->m_loffli) loFFLi;
    EXPECT_THAT(loFFLi.push(0), Eq(false));
}
} // namespace
