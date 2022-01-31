// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "test_cxx_functional_interface_types.hpp"

namespace
{
using namespace test_cxx_functional_interface;
using namespace ::testing;

constexpr bool TYPE_HAS_VALUE_METHOD = true;
constexpr bool TYPE_HAS_NO_VALUE_METHOD = false;

#define IOX_TEST(TestName, variationPoint)                                                                             \
    using SutType = typename TestFixture::TestFactoryType::Type;                                                       \
    constexpr bool HAS_VALUE_METHOD = iox::cxx::internal::HasValueMethod<SutType>::value;                              \
    ValueOrReturnsValueWhenValid<HAS_VALUE_METHOD>::template performTest<typename TestFixture::TestFactoryType>(       \
        [](auto& sut, auto alternativeValue) { return variationPoint.value_or(alternativeValue); });


template <bool HasValue>
struct ValueOrReturnsValueWhenValid;

template <>
struct ValueOrReturnsValueWhenValid<TYPE_HAS_NO_VALUE_METHOD>
{
    template <typename TestFactory, typename ValueOrCall>
    static void performTest(const ValueOrCall&)
    {
    }
};

template <>
struct ValueOrReturnsValueWhenValid<TYPE_HAS_VALUE_METHOD>
{
    template <typename TestFactory, typename ValueOrCall>
    static void performTest(const ValueOrCall& callValueOr)
    {
        auto sut = TestFactory::createValidObject();
        EXPECT_THAT(callValueOr(sut, TestFactory::anotherTestValue), Eq(TestFactory::usedTestValue));
    }
};

TYPED_TEST(FunctionalInterface_test, ValueOrReturnsValueWhenValid_LValue)
{
    IOX_TEST(ValueOrReturnsValueWhenValid, sut);
}

TYPED_TEST(FunctionalInterface_test, ValueOrReturnsValueWhenValid_RValue)
{
    IOX_TEST(ValueOrReturnsValueWhenValid, std::move(sut));
}

template <bool HasValue>
struct ValueOrReturnsArgumentWhenInalid;

template <>
struct ValueOrReturnsArgumentWhenInalid<TYPE_HAS_NO_VALUE_METHOD>
{
    template <typename TestFactory, typename ValueOrCall>
    static void performTest(const ValueOrCall&)
    {
    }
};

template <>
struct ValueOrReturnsArgumentWhenInalid<TYPE_HAS_VALUE_METHOD>
{
    template <typename TestFactory, typename ValueOrCall>
    static void performTest(const ValueOrCall& callValueOr)
    {
        auto sut = TestFactory::createInvalidObject();
        EXPECT_THAT(callValueOr(sut, TestFactory::anotherTestValue), Eq(TestFactory::anotherTestValue));
    }
};

TYPED_TEST(FunctionalInterface_test, ValueOrReturnsArgumentWhenInalid_LValue)
{
    IOX_TEST(ValueOrReturnsArgumentWhenInalid, sut);
}

TYPED_TEST(FunctionalInterface_test, ValueOrReturnsArgumentWhenInalid_RValue)
{
    IOX_TEST(ValueOrReturnsArgumentWhenInalid, std::move(sut));
}

#undef IOX_TEST
} // namespace
