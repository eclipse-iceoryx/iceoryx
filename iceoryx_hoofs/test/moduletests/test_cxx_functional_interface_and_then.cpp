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

template <bool HasValue>
struct AndThenIsCalledCorrectlyWhenValid;

template <>
struct AndThenIsCalledCorrectlyWhenValid<false>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& andThenCall)
    {
        auto sut = TestFactory::createValidObject();
        bool wasCallbackCalled = false;
        andThenCall(sut, [&] { wasCallbackCalled = true; });
        EXPECT_TRUE(wasCallbackCalled);
    }
};

template <>
struct AndThenIsCalledCorrectlyWhenValid<true>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& andThenCall)
    {
        auto sut = TestFactory::createValidObject();
        bool wasCallbackCalled = false;
        andThenCall(sut, [&](auto& arg) {
            wasCallbackCalled = true;
            EXPECT_EQ(arg, TestFactory::usedTestValue);
        });
        EXPECT_TRUE(wasCallbackCalled);
    }
};

TYPED_TEST(FunctionalInterface_test, AndThenIsCalledCorrectlyWhenValid_LValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenIsCalledCorrectlyWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { sut.and_then(callback); });
}

TYPED_TEST(FunctionalInterface_test, AndThenIsCalledCorrectlyWhenValid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenIsCalledCorrectlyWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut, auto callback) { const_cast<const SutType&>(sut).and_then(callback); });
}

TYPED_TEST(FunctionalInterface_test, AndThenIsCalledCorrectlyWhenValid_RValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenIsCalledCorrectlyWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { std::move(sut).and_then(callback); });
}

TYPED_TEST(FunctionalInterface_test, AndThenIsCalledCorrectlyWhenValid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenIsCalledCorrectlyWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut, auto callback) { std::move(const_cast<const SutType&>(sut)).and_then(callback); });
}

template <bool HasValue>
struct AndThenIsNotCalledWhenInvalid;

template <>
struct AndThenIsNotCalledWhenInvalid<false>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& andThenCall)
    {
        auto sut = TestFactory::createInvalidObject();
        bool wasCallbackCalled = false;
        andThenCall(sut, [&] { wasCallbackCalled = true; });
        EXPECT_FALSE(wasCallbackCalled);
    }
};

template <>
struct AndThenIsNotCalledWhenInvalid<true>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& andThenCall)
    {
        auto sut = TestFactory::createInvalidObject();
        bool wasCallbackCalled = false;
        andThenCall(sut, [&](auto&) { wasCallbackCalled = true; });
        EXPECT_FALSE(wasCallbackCalled);
    }
};

TYPED_TEST(FunctionalInterface_test, AndThenIsNotCalledWhenInvalid_LValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenIsNotCalledWhenInvalid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { sut.and_then(callback); });
}

TYPED_TEST(FunctionalInterface_test, AndThenIsNotCalledWhenInvalid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenIsNotCalledWhenInvalid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut, auto callback) { const_cast<const SutType&>(sut).and_then(callback); });
}

TYPED_TEST(FunctionalInterface_test, AndThenIsNotCalledWhenInvalid_RValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenIsNotCalledWhenInvalid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { std::move(sut).and_then(callback); });
}

TYPED_TEST(FunctionalInterface_test, AndThenIsNotCalledWhenInvalid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenIsNotCalledWhenInvalid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut, auto callback) { std::move(const_cast<const SutType&>(sut)).and_then(callback); });
}
} // namespace
