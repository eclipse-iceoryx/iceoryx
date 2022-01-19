// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI. All rights reserved.
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

#include "iceoryx_hoofs/cxx/attributes.hpp"
#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox::popo;
using namespace iox::cxx;

TEST(TypedUniqueId_RouDiId, SettingTheRouDiIdWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "473467bf-1a6f-4cd2-acd8-447a623a5301");
    uint16_t someId = 1243u;
    iox::popo::internal::setUniqueRouDiId(someId);
    EXPECT_EQ(iox::popo::internal::getUniqueRouDiId(), someId);
    iox::popo::internal::unsetUniqueRouDiId();
}

TEST(TypedUniqueId_RouDiId, SettingTheRouDiIdTwiceFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "fe468314-cd38-4363-bbf9-f106bf9ec1f4");
    uint16_t someId = 1243u;
    bool errorHandlerCalled = false;
    auto errorHandlerGuard = iox::ErrorHandler<iox::PoshError>::setTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::PoshError error IOX_MAYBE_UNUSED,
                              const std::function<void()>,
                              const iox::ErrorLevel) { errorHandlerCalled = true; });

    iox::popo::internal::setUniqueRouDiId(someId);
    EXPECT_FALSE(errorHandlerCalled);
    iox::popo::internal::setUniqueRouDiId(someId);
    EXPECT_TRUE(errorHandlerCalled);
    iox::popo::internal::unsetUniqueRouDiId();
}

TEST(TypedUniqueId_RouDiId, GettingTheRouDiIdWithoutSettingFails)
{
    ::testing::Test::RecordProperty("TEST_ID", "68de213f-7009-4573-8791-9f09f8ba413c");
    bool errorHandlerCalled = false;
    auto errorHandlerGuard = iox::ErrorHandler<iox::PoshError>::setTemporaryErrorHandler(
        [&errorHandlerCalled](const iox::PoshError error IOX_MAYBE_UNUSED,
                              const std::function<void()>,
                              const iox::ErrorLevel) { errorHandlerCalled = true; });

    iox::popo::internal::getUniqueRouDiId();
    EXPECT_TRUE(errorHandlerCalled);
}

template <typename T>
class TypedUniqueId_test : public Test
{
  protected:
    using UniqueIDType = T;

    iox::cxx::GenericRAII m_uniqueRouDiId{[] { iox::popo::internal::setUniqueRouDiId(0); },
                                          [] { iox::popo::internal::unsetUniqueRouDiId(); }};
};

using Implementations = Types<TypedUniqueId<int>, TypedUniqueId<float>>;

TYPED_TEST_SUITE(TypedUniqueId_test, Implementations);


TYPED_TEST(TypedUniqueId_test, DefaultConstructorIncrementsID)
{
    ::testing::Test::RecordProperty("TEST_ID", "2912c5bb-fd6d-46b4-ab32-97cfb7018860");
    typename TestFixture::UniqueIDType a, b;
    EXPECT_THAT(static_cast<uint64_t>(a) + 1, Eq(static_cast<uint64_t>(b)));
}

TYPED_TEST(TypedUniqueId_test, CopyConstructorSetsSameID)
{
    ::testing::Test::RecordProperty("TEST_ID", "dbe1a4fd-f4fe-47e5-83ef-38a9e59afd94");
    typename TestFixture::UniqueIDType a, b(a);
    EXPECT_THAT(static_cast<uint64_t>(a), Eq(static_cast<uint64_t>(b)));
}

TYPED_TEST(TypedUniqueId_test, CopyConstructorAssignmentSetsSameID)
{
    ::testing::Test::RecordProperty("TEST_ID", "31dfad29-0da0-41b8-b78f-dbeda0d7e684");
    typename TestFixture::UniqueIDType a, b;
    a = b;
    EXPECT_THAT(a, Eq(b));
}

TYPED_TEST(TypedUniqueId_test, MoveConstructorSetsSameID)
{
    ::testing::Test::RecordProperty("TEST_ID", "2abb1afa-094a-4d28-a3cc-328212ad2f7b");
    typename TestFixture::UniqueIDType a;
    auto id = static_cast<uint64_t>(a);
    decltype(a) b(std::move(a));
    EXPECT_THAT(static_cast<uint64_t>(b), Eq(id));
}

TYPED_TEST(TypedUniqueId_test, MoveAssignmentSetsSameID)
{
    ::testing::Test::RecordProperty("TEST_ID", "5d8a8771-be7a-45d2-b2bb-be89938241b6");
    typename TestFixture::UniqueIDType a, b;
    auto id = static_cast<uint64_t>(a);
    b = std::move(a);
    EXPECT_THAT(static_cast<uint64_t>(b), Eq(id));
}

TYPED_TEST(TypedUniqueId_test, SameIDsAreEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "4040fe82-3220-402d-8618-b152f6c1042e");
    typename TestFixture::UniqueIDType a, b(a);
    EXPECT_TRUE(a == b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a >= b);
}

TYPED_TEST(TypedUniqueId_test, DifferentIDsAreNotEqual)
{
    ::testing::Test::RecordProperty("TEST_ID", "ca8c7eae-d2af-4560-9bb7-c2e876103a62");
    typename TestFixture::UniqueIDType a, b;
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(a != b);
    EXPECT_FALSE(a >= b);
}

TYPED_TEST(TypedUniqueId_test, LatestIDIsGreatestID)
{
    ::testing::Test::RecordProperty("TEST_ID", "1327e92a-bc07-4ec9-8bc5-ee5a935ccd66");
    typename TestFixture::UniqueIDType a, b;
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a <= b);
    EXPECT_FALSE(a > b);
    EXPECT_FALSE(a >= b);
}

TYPED_TEST(TypedUniqueId_test, FirstIDIsSmallestID)
{
    ::testing::Test::RecordProperty("TEST_ID", "6951e91f-0112-4c97-b972-34ddeada4191");
    typename TestFixture::UniqueIDType a, b;
    EXPECT_FALSE(b < a);
    EXPECT_FALSE(b <= a);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(b >= a);
}

TYPED_TEST(TypedUniqueId_test, ConversionToUint64)
{
    ::testing::Test::RecordProperty("TEST_ID", "c1c58736-9755-4736-b163-3c8cc26db80d");
    typename TestFixture::UniqueIDType a, b;
    uint64_t id = static_cast<uint64_t>(a);
    b = a;
    EXPECT_EQ(id, static_cast<uint64_t>(b));
}

TYPED_TEST(TypedUniqueId_test, CreatingAnUniqueIdWithDefaultCTorIsValid)
{
    ::testing::Test::RecordProperty("TEST_ID", "0d810ee1-8ddd-48b3-8b53-d4430dd4bbe6");
    typename TestFixture::UniqueIDType a;
    EXPECT_TRUE(a.isValid());
}

TYPED_TEST(TypedUniqueId_test, InvalidIdIsInvalid)
{
    ::testing::Test::RecordProperty("TEST_ID", "d0576b8d-65d2-4b53-88c0-05078f434a41");
    typename TestFixture::UniqueIDType a(InvalidId);
    EXPECT_FALSE(a.isValid());
}

} // namespace
