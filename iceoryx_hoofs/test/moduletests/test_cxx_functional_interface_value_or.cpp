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

template <bool HasValue>
struct ValueOrReturnsValueWhenValid;

template <>
struct ValueOrReturnsValueWhenValid<TYPE_HAS_NO_VALUE_METHOD>
{
    template <typename TestFactory>
    static void performTest()
    {
    }
};

template <>
struct ValueOrReturnsValueWhenValid<TYPE_HAS_VALUE_METHOD>
{
    template <typename TestFactory>
    static void performTest()
    {
        auto sut = TestFactory::createValidObject();
        EXPECT_THAT(sut.value_or(TestFactory::anotherTestValue), Eq(TestFactory::usedTestValue));
    }
};

TYPED_TEST(FunctionalInterface_test, ValueOrReturnsValueWhenValid)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ValueOrReturnsValueWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>();
}

template <bool HasValue>
struct ValueOrReturnsArgumentWhenInalid;

template <>
struct ValueOrReturnsArgumentWhenInalid<TYPE_HAS_NO_VALUE_METHOD>
{
    template <typename TestFactory>
    static void performTest()
    {
    }
};

template <>
struct ValueOrReturnsArgumentWhenInalid<TYPE_HAS_VALUE_METHOD>
{
    template <typename TestFactory>
    static void performTest()
    {
        auto sut = TestFactory::createInvalidObject();
        EXPECT_THAT(sut.value_or(TestFactory::anotherTestValue), Eq(TestFactory::anotherTestValue));
    }
};

TYPED_TEST(FunctionalInterface_test, ValueOrReturnsArgumentWhenInalid)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ValueOrReturnsArgumentWhenInalid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>();
}

} // namespace
