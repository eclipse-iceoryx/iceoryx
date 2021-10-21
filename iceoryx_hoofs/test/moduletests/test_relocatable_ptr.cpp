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

#include "iceoryx_hoofs/internal/relocatable_pointer/relocatable_ptr.hpp"

#include "test.hpp"

#include <cstring>
#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox::rp;

// Needed especially for void implementation tests where we cannot
// construct a corresponding object of type void to point to.
template <typename T>
inline T* nonNullPtr()
{
    static T t{};
    return &t;
}

template <>
inline void* nonNullPtr<void>()
{
    static int t;
    return &t;
}

template <>
inline const void* nonNullPtr<const void>()
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
inline void* otherNonNullPtr<void>()
{
    static int t;
    return &t;
}

template <>
inline const void* otherNonNullPtr<const void>()
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
};

// does the conversion operator of P return a T?
template <typename P, typename T>
constexpr auto conversionReturns()
{
    using R = decltype(std::declval<P>().operator T());
    return std::is_same<R, T>::value;
};

// does the arrow operator of P return a T?
template <typename P, typename T>
constexpr auto arrowReturns()
{
    using R = decltype(std::declval<P>().operator->());
    return std::is_same<R, T>::value;
};

// does the dereferencing operator of P return a T?
template <typename P, typename T>
constexpr auto dereferencingReturns()
{
    using R = decltype(std::declval<P>().operator*());
    return std::is_same<R, T>::value;
};


struct Data
{
    Data(uint32_t value = 0)
        : value(value)
    {
    }
    uint32_t value;
};

class RelocatableType
{
  public:
    RelocatableType(int value)
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
    iox::rp::relocatable_ptr<int> rp;
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
        internal::CaptureStderr();
    }

    void TearDown() override
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }

    using DataType = T;
};

// Other tests not specific to a template type
class Relocatable_ptr_test : public Test
{
  public:
    void SetUp() override
    {
        internal::CaptureStderr();
    }

    void TearDown() override
    {
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
};


typedef ::testing::Types<int, Data, void, char*, const Data, const void> TestTypes;

/// we require TYPED_TEST since we support gtest 1.8 for our safety targets
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TYPED_TEST_CASE(Relocatable_ptr_typed_test, TestTypes);
#pragma GCC diagnostic pop

TYPED_TEST(Relocatable_ptr_typed_test, wrappedPointerTypeIsCorrect)
{
    using T = typename TestFixture::DataType;
    using P = typename iox::rp::relocatable_ptr<T>::ptr_t;
    constexpr bool pointerTypeIsCorrect = std::is_same<P, T*>::value;
    EXPECT_TRUE(pointerTypeIsCorrect);
}

TYPED_TEST(Relocatable_ptr_typed_test, defaulCtorCreatesNullpointer)
{
    using T = typename TestFixture::DataType;
    iox::rp::relocatable_ptr<T> rp;
    EXPECT_EQ(rp.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, copyCtorOfNullptrWorks)
{
    using T = typename TestFixture::DataType;
    iox::rp::relocatable_ptr<T> rp1;
    iox::rp::relocatable_ptr<T> rp2(rp1);
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, moveCtorOfNullptrWorks)
{
    using T = typename TestFixture::DataType;
    iox::rp::relocatable_ptr<T> rp1;
    iox::rp::relocatable_ptr<T> rp2(std::move(rp1));
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, copyAssignmentOfNullptrWorks)
{
    using T = typename TestFixture::DataType;
    // we cannot construct an actual object if T = void
    // and it is not necessary fro most tests, we just need some non-nullptr
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1;
    iox::rp::relocatable_ptr<T> rp2(p);
    rp2 = rp1;
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, moveAssignmentOfNullptrWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1;
    iox::rp::relocatable_ptr<T> rp2(p);
    rp2 = std::move(rp1);
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), nullptr);
}

TYPED_TEST(Relocatable_ptr_typed_test, nonNullPointerConstructionWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp(p);
    EXPECT_EQ(rp.get(), p);
}

TYPED_TEST(Relocatable_ptr_typed_test, copyCtorWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1(p);
    iox::rp::relocatable_ptr<T> rp2(rp1);
    EXPECT_EQ(rp1.get(), p);
    EXPECT_EQ(rp2.get(), p);
}

TYPED_TEST(Relocatable_ptr_typed_test, moveCtorWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1(p);
    iox::rp::relocatable_ptr<T> rp2(std::move(rp1));
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), p);
}

TYPED_TEST(Relocatable_ptr_typed_test, copyAssignmentWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1(p);
    iox::rp::relocatable_ptr<T> rp2;
    rp2 = rp1;
    EXPECT_EQ(rp1.get(), p);
    EXPECT_EQ(rp2.get(), p);
}

TYPED_TEST(Relocatable_ptr_typed_test, moveAssignmentWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1(p);
    iox::rp::relocatable_ptr<T> rp2;
    rp2 = std::move(rp1);
    EXPECT_EQ(rp1.get(), nullptr);
    EXPECT_EQ(rp2.get(), p);
}

// regular get is tested with the ctor
TYPED_TEST(Relocatable_ptr_typed_test, constGetWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    const iox::rp::relocatable_ptr<T> rp(p);
    EXPECT_EQ(rp.get(), p);

    constexpr bool isConst = getReturns<decltype(rp), const T*>();
    EXPECT_TRUE(isConst);
}

TYPED_TEST(Relocatable_ptr_typed_test, conversionToRawPointerWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp(p);
    T* q = rp;
    EXPECT_EQ(q, p);

    constexpr bool isNotConst = conversionReturns<decltype(rp), T*>();
    EXPECT_TRUE(isNotConst);
}

TYPED_TEST(Relocatable_ptr_typed_test, conversionToConstRawPointerWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    const iox::rp::relocatable_ptr<T> rp(p);
    const T* q = rp;
    EXPECT_EQ(q, p);

    constexpr bool isConst = conversionReturns<decltype(rp), const T*>();
    EXPECT_TRUE(isConst);
}

TYPED_TEST(Relocatable_ptr_typed_test, arrowOperatorWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp(p);
    EXPECT_EQ(rp.operator->(), p);

    constexpr bool isNotConst = arrowReturns<decltype(rp), T*>();
    EXPECT_TRUE(isNotConst);
}

TYPED_TEST(Relocatable_ptr_typed_test, arrowOperatorConstWorks)
{
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    const iox::rp::relocatable_ptr<T> rp(p);
    EXPECT_EQ(rp.operator->(), p);

    constexpr bool isConst = arrowReturns<decltype(rp), const T*>();
    EXPECT_TRUE(isConst);
}


TYPED_TEST(Relocatable_ptr_typed_test, nullptrIsEqualToNullptr)
{
    using T = typename TestFixture::DataType;
    iox::rp::relocatable_ptr<T> rp1;
    iox::rp::relocatable_ptr<T> rp2;

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
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1(p);
    iox::rp::relocatable_ptr<T> rp2;

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
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1(p);
    iox::rp::relocatable_ptr<T> rp2(p);

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
    using T = typename TestFixture::DataType;
    T* p1 = nonNullPtr<T>();
    T* p2 = otherNonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp1(p1);
    iox::rp::relocatable_ptr<T> rp2(p2);

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
    using T = typename TestFixture::DataType;
    T* p = nonNullPtr<T>();
    iox::rp::relocatable_ptr<T> rp(p);

    if (rp)
    {
        SUCCEED();
    }
    else
    {
        FAIL();
    }
}

TYPED_TEST(Relocatable_ptr_typed_test, positiveNullPointerCheckWithIfWorks)
{
    using T = typename TestFixture::DataType;
    iox::rp::relocatable_ptr<T> rp;

    if (rp)
    {
        FAIL();
    }
    else
    {
        SUCCEED();
    }
}

TEST_F(Relocatable_ptr_test, dereferencingWorks)
{
    int x = 666;
    iox::rp::relocatable_ptr<int> rp(&x);
    EXPECT_EQ(*rp, x);

    constexpr bool isNotConst = dereferencingReturns<decltype(rp), int&>();
    EXPECT_TRUE(isNotConst);
}

TEST_F(Relocatable_ptr_test, dereferencingConstWorks)
{
    int x = 314;
    const iox::rp::relocatable_ptr<int> rp(&x);
    EXPECT_EQ(*rp, x);

    constexpr bool isConst = dereferencingReturns<decltype(rp), const int&>();
    EXPECT_TRUE(isConst);
}

TEST_F(Relocatable_ptr_test, dereferencingComplexTypeWorks)
{
    Data x(69);
    iox::rp::relocatable_ptr<Data> rp(&x);
    EXPECT_EQ((*rp).value, x.value);
    EXPECT_EQ(rp->value, x.value);
}

TEST_F(Relocatable_ptr_test, dereferencingConstComplexTypeWorks)
{
    Data x(42);
    const iox::rp::relocatable_ptr<Data> rp(&x);
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
    using T = RelocatableType;
    using storage_t = std::aligned_storage<sizeof(T), alignof(T)>::type;
    storage_t sourceStorage, destStorage;

    void* sourcePtr = new (&sourceStorage) T(37);
    void* destPtr = &destStorage;
    T* source = reinterpret_cast<T*>(sourcePtr);
    T* dest = reinterpret_cast<T*>(destPtr);

    EXPECT_EQ(source->data, 37);
    EXPECT_EQ(*source->rp, 37);

    // structure is relocated by memcopy
    std::memcpy(destPtr, sourcePtr, sizeof(T));
    source->clear();

    EXPECT_EQ(source->data, 0);
    EXPECT_EQ(dest->data, 37);

    // points to relocated data automatically
    EXPECT_EQ(*dest->rp, 37);
    dest->data = 73;

    EXPECT_EQ(source->data, 0);
    EXPECT_EQ(*dest->rp, 73);

    source->~T();
    dest->~T();
}

} // namespace
