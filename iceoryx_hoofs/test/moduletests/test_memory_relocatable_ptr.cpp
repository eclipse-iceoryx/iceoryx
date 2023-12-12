// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iox/relocatable_ptr.hpp"

#include "test.hpp"

#include <cstring>
#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox;

// Needed especially for void implementation tests where we cannot
// construct a corresponding object of type void to point to.
template <typename T>
inline T* nonNullPtr()
{
    static T t{};
    return &t;
}

template <>
[[maybe_unused]] inline void* nonNullPtr<void>()
{
    static int t;
    return &t;
}

template <>
[[maybe_unused]] inline const void* nonNullPtr<const void>()
{
    static int t;
    return &t;
}

template <typename T>
inline T* otherNonNullPtr()
{
    static T t{};
    return &t;
}

template <>
[[maybe_unused]] inline void* otherNonNullPtr<void>()
{
    static int t;
    return &t;
}

template <>
[[maybe_unused]] inline const void* otherNonNullPtr<const void>()
{
    static int t;
    return &t;
}

// does P wrap a T?
template <typename P, typename T>
constexpr auto getReturns()
{
    using R = decltype(std::declval<P>().get());
    return std::is_same<R, T>::value;
}

// does the conversion operator of P return a T?
template <typename P, typename T>
constexpr auto conversionReturns()
{
    using R = decltype(std::declval<P>().operator T());
    return std::is_same<R, T>::value;
}

// does the arrow operator of P return a T?
template <typename P, typename T>
constexpr auto arrowReturns()
{
    using R = decltype(std::declval<P>().operator->());
    return std::is_same<R, T>::value;
}

// does the dereferencing operator of P return a T?
template <typename P, typename T>
constexpr auto dereferencingReturns()
{
    using R = decltype(std::declval<P>().operator*());
    return std::is_same<R, T>::value;
}


struct Data
{
    explicit Data(uint32_t value = 0)
        : value(value)
    {
    }
    uint32_t value;
};

class RelocatableType
{
  public:
    explicit RelocatableType(int value)
        : data(value)
        , rp(&this->data)
    {
    }

    void clear()
    {
        void* p = this; // make opaque to suppress warning
        // the memset is safe since it is operating on primitive types
        std::memset(p, 0, sizeof(RelocatableType));
    }

    RelocatableType(const RelocatableType&) = delete;
    RelocatableType& operator=(const RelocatableType&) = delete;
    RelocatableType(RelocatableType&&) = delete;
    RelocatableType& operator=(RelocatableType&&) = delete;

    int data;
    relocatable_ptr<int> rp;
};

// Not all tests make sense to be run as typed tests
// due to interface / behaviour differences,
// e.g. dereferencing for void

// Tests for all template types
template <typename T>
class Relocatable_ptr_typed_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    using DataType = T;
};

// Other tests not specific to a template type
class Relocatable_ptr_test : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};


typedef ::testing::Types<int, Data, void, char*, const Data, const void> TestTypes;

TYPED_TEST_SUITE(Relocatable_ptr_typed_test, TestTypes, );

TYPED_TEST(Relocatable_ptr_typed_test, wrappedPointerTypeIsCorrect)
{
    ::testing::Test::RecordProperty("TEST_ID", "12de29e3-673c-487c-9808-67e5c3e25c73");
    using T = typename TestFixture::DataType;
    using P = typename relocatable_ptr<T>::ptr_t;
    constexpr bool pointerTypeIsCorrect = std::is_same<P, T*>::value;
    EXPECT_TRUE(pointerTypeIsCorrect);
}

TYPED_TEST(Relocatable_ptr_typed_test, defaulCtorCreatesNullpointer)
{
    ::testing::Test::RecordProperty("TEST_ID", "6823533f-9594-4f53-9493-d80a73706013");
    using T = typename TestFixture::DataType;
    relocatable_ptr<T> rp;
    EXPECT_EQ(rp.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, copyCtorOfNullptrWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "de23e14d-1c06-4005-ba61-45e5aeedaf47");
    using T = typename TestFixture::DataType;
    relocatable_ptr<T> rp1;
    relocatable_ptr<T> rp2(rp1);
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, moveCtorOfNullptrWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "f0ecd49e-c165-4e25-985c-5bc44a072f2e");
    using T = typename TestFixture::DataType;
    relocatable_ptr<T> rp1;
    relocatable_ptr<T> rp2(std::move(rp1));
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved pointer
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, copyAssignmentOfNullptrWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "635665f9-70a1-405e-981c-155c0f2f81a7");
    using T = typename TestFixture::DataType;
    // we cannot construct an actual object if T = void
    // and it is not necessary fro most tests, we just need some non-nullptr
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp1;
    relocatable_ptr<T> rp2(p);
    rp2 = rp1;
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, moveAssignmentOfNullptrWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b15da71c-bb71-4059-a3fb-7d5d8f8020a6");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp1;
    relocatable_ptr<T> rp2(p);
    rp2 = std::move(rp1);
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved pointer
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, nonNullPointerConstructionWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6258b81c-97b4-4d5b-9543-ca7e2fc8e6f0");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp(p);
    EXPECT_EQ(rp.get(), p);
}

TYPED_TEST(Relocatable_ptr_typed_test, copyCtorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "453b6c32-e5ba-4b15-86af-d7601ee5b97e");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp1(p);
    relocatable_ptr<T> rp2(rp1);
    EXPECT_EQ(rp1.get(), p);
    EXPECT_EQ(rp2.get(), p);
}

TYPED_TEST(Relocatable_ptr_typed_test, moveCtorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "42c3a34d-4dc8-4bf6-bf1e-1fc742570ee2");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp1(p);
    relocatable_ptr<T> rp2(std::move(rp1));
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved pointer
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), p);
}

TYPED_TEST(Relocatable_ptr_typed_test, copyAssignmentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "431354d5-400d-49cd-8554-4ee797661cf7");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp1(p);
    relocatable_ptr<T> rp2;
    rp2 = rp1;
    EXPECT_EQ(rp1.get(), p);
    EXPECT_EQ(rp2.get(), p);
}

TYPED_TEST(Relocatable_ptr_typed_test, moveAssignmentWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "5041782e-2c57-4739-ab21-2d7c1dfc3991");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp1(p);
    relocatable_ptr<T> rp2;
    rp2 = std::move(rp1);
    // NOLINTJUSTIFICATION we explicitly want to test the defined state of a moved pointer
    // NOLINTNEXTLINE(bugprone-use-after-move,hicpp-invalid-access-moved,clang-analyzer-cplusplus.Move)
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), p);
}

// regular get is tested with the ctor
TYPED_TEST(Relocatable_ptr_typed_test, constGetWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1b478221-d1fb-44aa-905e-7f40d961eaff");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    const relocatable_ptr<T> rp(p);
    EXPECT_EQ(rp.get(), p);

    constexpr bool isConst = getReturns<decltype(rp), const T*>();
    EXPECT_TRUE(isConst);
}

TYPED_TEST(Relocatable_ptr_typed_test, conversionToRawPointerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "d7e46b17-62bc-4b05-a827-b91d2a4b9f23");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp(p);
    T* q = rp;
    EXPECT_EQ(q, p);

    constexpr bool isNotConst = conversionReturns<decltype(rp), T*>();
    EXPECT_TRUE(isNotConst);
}

TYPED_TEST(Relocatable_ptr_typed_test, conversionToConstRawPointerWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "82f40725-67d9-4f80-812a-1fcbdd1cbf60");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    const relocatable_ptr<T> rp(p);
    const T* q = rp;
    EXPECT_EQ(q, p);

    constexpr bool isConst = conversionReturns<decltype(rp), const T*>();
    EXPECT_TRUE(isConst);
}

TYPED_TEST(Relocatable_ptr_typed_test, arrowOperatorWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "6f3a7428-fd7b-4a98-b3ce-90ac73655ac8");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp(p);
    EXPECT_EQ(rp.operator->(), p);

    constexpr bool isNotConst = arrowReturns<decltype(rp), T*>();
    EXPECT_TRUE(isNotConst);
}

TYPED_TEST(Relocatable_ptr_typed_test, arrowOperatorConstWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "38bf2bb7-6550-4534-b0fe-6ec07632c6d3");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    const relocatable_ptr<T> rp(p);
    EXPECT_EQ(rp.operator->(), p);

    constexpr bool isConst = arrowReturns<decltype(rp), const T*>();
    EXPECT_TRUE(isConst);
}


TYPED_TEST(Relocatable_ptr_typed_test, nullptrIsEqualToNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "45d24a0b-5a46-4a10-bbb8-7f8b7647a992");
    using T = typename TestFixture::DataType;
    relocatable_ptr<T> rp1;
    relocatable_ptr<T> rp2;

    EXPECT_TRUE(rp1 == rp2);
    EXPECT_TRUE(rp2 == rp1);
    EXPECT_TRUE(rp1 == nullptr);
    EXPECT_TRUE(nullptr == rp2);

    EXPECT_FALSE(rp1 != rp2);
    EXPECT_FALSE(rp2 != rp1);
    EXPECT_FALSE(rp1 != nullptr);
    EXPECT_FALSE(nullptr != rp2);
}

TYPED_TEST(Relocatable_ptr_typed_test, nullptrIsNotEqualToNonNullptr)
{
    ::testing::Test::RecordProperty("TEST_ID", "a3fd804d-9e53-4fef-9b34-bb43bd778c02");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp1(p);
    relocatable_ptr<T> rp2;

    EXPECT_FALSE(rp1 == rp2);
    EXPECT_FALSE(rp2 == rp1);
    EXPECT_FALSE(rp1 == nullptr);
    EXPECT_FALSE(nullptr == rp1);

    EXPECT_TRUE(rp1 != rp2);
    EXPECT_TRUE(rp2 != rp1);
    EXPECT_TRUE(rp1 != nullptr);
    EXPECT_TRUE(nullptr != rp1);
}

TYPED_TEST(Relocatable_ptr_typed_test, equalNonNullptrComparisonWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "c3ed8892-db76-4d62-8c04-51819696c7dc");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp1(p);
    relocatable_ptr<T> rp2(p);

    EXPECT_TRUE(rp1 == rp2);
    EXPECT_TRUE(rp2 == rp1);
    EXPECT_TRUE(rp1 == p);
    EXPECT_TRUE(p == rp2);

    EXPECT_FALSE(rp1 != rp2);
    EXPECT_FALSE(rp2 != rp1);
    EXPECT_FALSE(p != rp2);
    EXPECT_FALSE(rp1 != p);
}

TYPED_TEST(Relocatable_ptr_typed_test, nonEqualNonNullptrComparisonWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "1cad3023-dd21-4242-a461-10f06cdea765");
    using T = typename TestFixture::DataType;
    T* p1 = nonNullPtr<T>();
    T* p2 = otherNonNullPtr<T>();
    relocatable_ptr<T> rp1(p1);
    relocatable_ptr<T> rp2(p2);

    EXPECT_FALSE(rp1 == rp2);
    EXPECT_FALSE(rp2 == rp1);
    EXPECT_FALSE(p1 == rp2);
    EXPECT_FALSE(rp1 == p2);

    EXPECT_TRUE(rp1 != rp2);
    EXPECT_TRUE(rp2 != rp1);
    EXPECT_TRUE(p1 != rp2);
    EXPECT_TRUE(rp1 != p2);
}

TYPED_TEST(Relocatable_ptr_typed_test, negativeNullPointerCheckWithIfWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "77225288-be1d-4105-b2fb-7f5f452cad89");
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    relocatable_ptr<T> rp(p);

    if (rp)
    {
        GTEST_SUCCEED();
    }
    else
    {
        GTEST_FAIL();
    }
}

TYPED_TEST(Relocatable_ptr_typed_test, positiveNullPointerCheckWithIfWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "4a57801b-3f52-4027-a2f1-474274e83515");
    using T = typename TestFixture::DataType;
    relocatable_ptr<T> rp;

    if (rp)
    {
        GTEST_FAIL();
    }
}

TEST_F(Relocatable_ptr_test, dereferencingWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "ea67f218-6ff8-4a82-a81e-52ae988546dc");
    constexpr int VALUE = 666;
    int x = VALUE;
    relocatable_ptr<int> rp(&x);
    EXPECT_EQ(*rp, VALUE);

    constexpr bool isNotConst = dereferencingReturns<decltype(rp), int&>();
    EXPECT_TRUE(isNotConst);
}

TEST_F(Relocatable_ptr_test, dereferencingConstWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "64a7e44e-b9eb-428a-bd50-3bd9e14400bc");
    constexpr int VALUE = 314;
    int x = VALUE;
    const relocatable_ptr<int> rp(&x);
    EXPECT_EQ(*rp, VALUE);

    constexpr bool isConst = dereferencingReturns<decltype(rp), const int&>();
    EXPECT_TRUE(isConst);
}

TEST_F(Relocatable_ptr_test, dereferencingComplexTypeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "e4a2bda1-c3f2-424e-b6dd-a4da6703b699");
    constexpr int VALUE = 69;
    Data x(VALUE);
    relocatable_ptr<Data> rp(&x);
    EXPECT_EQ((*rp).value, x.value);
    EXPECT_EQ(rp->value, x.value);
}

TEST_F(Relocatable_ptr_test, dereferencingConstComplexTypeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b60f0fd5-ff9b-40a5-ad0d-d13965eff578");
    constexpr int VALUE = 69;
    Data x(VALUE);
    const relocatable_ptr<Data> rp(&x);
    EXPECT_EQ((*rp).value, x.value);
    EXPECT_EQ(rp->value, x.value);
}

// Checks whether copying a structure ontaining a relocatable_ptr
// to internal data works as expected.
// This means that the structure is properly copied by memcpy
// and the copy works at the destination and does not depend on the source.
// To verify this, we set the source to 0 after the copy.
TEST_F(Relocatable_ptr_test, relocationWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b1b85836-2a4f-4859-a8f9-796e20fbb735");
    using T = RelocatableType;
    using storage_t = std::aligned_storage<sizeof(T), alignof(T)>::type;
    storage_t sourceStorage;
    storage_t destStorage;

    constexpr uint32_t SOURCE_VALUE{37};
    constexpr uint32_t NEW_VALUE{73};

    void* sourcePtr = new (&sourceStorage) T(SOURCE_VALUE);
    void* destPtr = &destStorage;
    // NOLINTJUSTIFICATION explicit cast to void* required for memcpy of relocatable type with copy ctor
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    T* source = reinterpret_cast<T*>(sourcePtr);
    T* dest = reinterpret_cast<T*>(destPtr);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

    EXPECT_EQ(source->data, SOURCE_VALUE);
    EXPECT_EQ(*source->rp, SOURCE_VALUE);

    // structure is relocated by memcopy
    std::memcpy(destPtr, sourcePtr, sizeof(T));
    source->clear();

    EXPECT_EQ(source->data, 0);
    EXPECT_EQ(dest->data, SOURCE_VALUE);

    // points to relocated data automatically
    EXPECT_EQ(*dest->rp, SOURCE_VALUE);
    dest->data = NEW_VALUE;

    EXPECT_EQ(source->data, 0);
    EXPECT_EQ(*dest->rp, NEW_VALUE);

    source->~T();
    dest->~T();
}

} // namespace
