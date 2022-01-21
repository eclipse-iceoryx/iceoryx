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

template <bool HasError>
struct OrElseIsCalledCorrectlyWhenInvalid;

template <>
struct OrElseIsCalledCorrectlyWhenInvalid<false>
{
    template <typename TestFactory, typename OrElseCall>
    static void performTest(const OrElseCall& orElseCall)
    {
        auto sut = TestFactory::createInvalidObject();
        bool wasCallbackCalled = false;
        orElseCall(sut, [&] { wasCallbackCalled = true; });
        EXPECT_TRUE(wasCallbackCalled);
    }
};

template <>
struct OrElseIsCalledCorrectlyWhenInvalid<true>
{
    template <typename TestFactory, typename OrElseCall>
    static void performTest(const OrElseCall& orElseCall)
    {
        auto sut = TestFactory::createInvalidObject();
        bool wasCallbackCalled = false;
        orElseCall(sut, [&](auto& arg) {
            wasCallbackCalled = true;
            EXPECT_EQ(arg, TestFactory::usedErrorValue);
        });
        EXPECT_TRUE(wasCallbackCalled);
    }
};

TYPED_TEST(FunctionalInterface_test, OrElseIsCalledCorrectlyWhenInvalid_LValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseIsCalledCorrectlyWhenInvalid<iox::cxx::internal::HasGetErrorMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { sut.or_else(callback); });
}

TYPED_TEST(FunctionalInterface_test, OrElseIsCalledCorrectlyWhenInvalid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseIsCalledCorrectlyWhenInvalid<iox::cxx::internal::HasGetErrorMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut, auto callback) { const_cast<const SutType&>(sut).or_else(callback); });
}

TYPED_TEST(FunctionalInterface_test, OrElseIsCalledCorrectlyWhenInvalid_RValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseIsCalledCorrectlyWhenInvalid<iox::cxx::internal::HasGetErrorMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { std::move(sut).or_else(callback); });
}

TYPED_TEST(FunctionalInterface_test, OrElseIsCalledCorrectlyWhenInvalid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseIsCalledCorrectlyWhenInvalid<iox::cxx::internal::HasGetErrorMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut, auto callback) { std::move(const_cast<const SutType&>(sut)).or_else(callback); });
}

template <bool HasError>
struct OrElseIsNotCalledWhenValid;

template <>
struct OrElseIsNotCalledWhenValid<false>
{
    template <typename TestFactory, typename OrElseCall>
    static void performTest(const OrElseCall& orElseCall)
    {
        auto sut = TestFactory::createValidObject();
        bool wasCallbackCalled = false;
        orElseCall(sut, [&] { wasCallbackCalled = true; });
        EXPECT_FALSE(wasCallbackCalled);
    }
};

template <>
struct OrElseIsNotCalledWhenValid<true>
{
    template <typename TestFactory, typename OrElseCall>
    static void performTest(const OrElseCall& orElseCall)
    {
        auto sut = TestFactory::createValidObject();
        bool wasCallbackCalled = false;
        orElseCall(sut, [&](auto&) { wasCallbackCalled = true; });
        EXPECT_FALSE(wasCallbackCalled);
    }
};

TYPED_TEST(FunctionalInterface_test, OrElseIsNotCalledWhenValid_LValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseIsNotCalledWhenValid<iox::cxx::internal::HasGetErrorMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { sut.or_else(callback); });
}

TYPED_TEST(FunctionalInterface_test, OrElseIsNotCalledWhenValid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseIsNotCalledWhenValid<iox::cxx::internal::HasGetErrorMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut, auto callback) { const_cast<const SutType&>(sut).or_else(callback); });
}

TYPED_TEST(FunctionalInterface_test, OrElseIsNotCalledWhenValid_RValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseIsNotCalledWhenValid<iox::cxx::internal::HasGetErrorMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut, auto callback) { std::move(sut).or_else(callback); });
}

TYPED_TEST(FunctionalInterface_test, OrElseIsNotCalledWhenValid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseIsNotCalledWhenValid<iox::cxx::internal::HasGetErrorMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut, auto callback) { std::move(const_cast<const SutType&>(sut)).or_else(callback); });
}


} // namespace
