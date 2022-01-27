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

#define IOX_TEST(TestName, variationPoint)                                                                             \
    using SutType = typename TestFixture::TestFactoryType::Type;                                                       \
    TestName<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<                                \
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { variationPoint.and_then(callback); })

constexpr bool TYPE_HAS_VALUE_METHOD = true;
constexpr bool TYPE_HAS_NO_VALUE_METHOD = false;

template <bool HasValue>
struct AndThenIsCalledCorrectlyWhenValid;

template <>
struct AndThenIsCalledCorrectlyWhenValid<TYPE_HAS_NO_VALUE_METHOD>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& callAndThen)
    {
        auto sut = TestFactory::createValidObject();
        bool wasCallbackCalled = false;
        auto andThenCallbackArgument = [&] { wasCallbackCalled = true; };
        callAndThen(sut, andThenCallbackArgument);
        EXPECT_TRUE(wasCallbackCalled);
    }
};

template <>
struct AndThenIsCalledCorrectlyWhenValid<TYPE_HAS_VALUE_METHOD>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& callAndThen)
    {
        auto sut = TestFactory::createValidObject();
        bool wasCallbackCalled = false;
        auto andThenCallbackArgument = [&](auto& arg) {
            wasCallbackCalled = true;
            EXPECT_EQ(arg, TestFactory::usedTestValue);
        };
        callAndThen(sut, andThenCallbackArgument);
        EXPECT_TRUE(wasCallbackCalled);
    }
};

TYPED_TEST(FunctionalInterface_test, AndThenIsCalledCorrectlyWhenValid_LValueCase)
{
    IOX_TEST(AndThenIsCalledCorrectlyWhenValid, sut);
}

TYPED_TEST(FunctionalInterface_test, AndThenIsCalledCorrectlyWhenValid_ConstLValueCase)
{
    IOX_TEST(AndThenIsCalledCorrectlyWhenValid, const_cast<const SutType&>(sut));
}

TYPED_TEST(FunctionalInterface_test, AndThenIsCalledCorrectlyWhenValid_RValueCase)
{
    IOX_TEST(AndThenIsCalledCorrectlyWhenValid, std::move(sut));
}

TYPED_TEST(FunctionalInterface_test, AndThenIsCalledCorrectlyWhenValid_ConstRValueCase)
{
    IOX_TEST(AndThenIsCalledCorrectlyWhenValid, std::move(const_cast<const SutType&>(sut)));
}

template <bool HasValue>
struct AndThenIsNotCalledWhenInvalid;

template <>
struct AndThenIsNotCalledWhenInvalid<TYPE_HAS_NO_VALUE_METHOD>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& callAndThen)
    {
        auto sut = TestFactory::createInvalidObject();
        bool wasCallbackCalled = false;
        auto andThenCallbackArgument = [&] { wasCallbackCalled = true; };
        callAndThen(sut, andThenCallbackArgument);
        EXPECT_FALSE(wasCallbackCalled);
    }
};

template <>
struct AndThenIsNotCalledWhenInvalid<TYPE_HAS_VALUE_METHOD>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& callAndThen)
    {
        auto sut = TestFactory::createInvalidObject();
        bool wasCallbackCalled = false;
        auto andThenCallbackArgument = [&](auto&) { wasCallbackCalled = true; };
        callAndThen(sut, andThenCallbackArgument);
        EXPECT_FALSE(wasCallbackCalled);
    }
};

TYPED_TEST(FunctionalInterface_test, AndThenIsNotCalledWhenInvalid_LValueCase)
{
    IOX_TEST(AndThenIsNotCalledWhenInvalid, sut);
}

TYPED_TEST(FunctionalInterface_test, AndThenIsNotCalledWhenInvalid_ConstLValueCase)
{
    IOX_TEST(AndThenIsNotCalledWhenInvalid, const_cast<const SutType&>(sut));
}

TYPED_TEST(FunctionalInterface_test, AndThenIsNotCalledWhenInvalid_RValueCase)
{
    IOX_TEST(AndThenIsNotCalledWhenInvalid, std::move(sut));
}

TYPED_TEST(FunctionalInterface_test, AndThenIsNotCalledWhenInvalid_ConstRValueCase)
{
    IOX_TEST(AndThenIsNotCalledWhenInvalid, std::move(const_cast<const SutType&>(sut)));
}

#undef IOX_TEST
} // namespace
