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
//
#include "iceoryx_hoofs/error_handling/error_handling.hpp"

#include "test_cxx_functional_interface_types.hpp"

namespace
{
using namespace test_cxx_functional_interface;
using namespace ::testing;

template <typename FactoryType, typename ExpectCall>
void ExpectDoesNotCallTerminateWhenObjectIsValid(const ExpectCall& expectCall)
{
    bool wasErrorHandlerCalled = false;
    auto sut = FactoryType::createValidObject();
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        expectCall(sut);
    }

    EXPECT_FALSE(wasErrorHandlerCalled);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_LValueCase)
{
    ExpectDoesNotCallTerminateWhenObjectIsValid<typename TestFixture::TestFactoryType>(
        [](auto& sut) { sut.expect("a seal on the head is better then a roof on a pidgin"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ExpectDoesNotCallTerminateWhenObjectIsValid<typename TestFixture::TestFactoryType>([](auto& sut) {
        const_cast<const SutType&>(sut).expect(
            "hypnotoad eats unicorns for breakfast - just kidding, hypnotoad would never harm another being");
    });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_RValueCase)
{
    ExpectDoesNotCallTerminateWhenObjectIsValid<typename TestFixture::TestFactoryType>(
        [](auto& sut) { std::move(sut).expect("hypnotoad is a friend of david hasselhof"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ExpectDoesNotCallTerminateWhenObjectIsValid<typename TestFixture::TestFactoryType>([](auto& sut) {
        std::move(const_cast<const SutType&>(sut)).expect("hypnotoads favorite animal is the leaf sheep");
    });
}

template <typename FactoryType, typename ExpectCall>
void ExpectDoesCallTerminateWhenObjectIsInvalid(const ExpectCall& expectCall)
{
    bool wasErrorHandlerCalled = true;
    auto sut = FactoryType::createInvalidObject();
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        expectCall(sut);
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_LValueCase)
{
    ExpectDoesCallTerminateWhenObjectIsInvalid<typename TestFixture::TestFactoryType>(
        [](auto& sut) { sut.expect("the chocolate rations will be increased soon"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_constLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ExpectDoesCallTerminateWhenObjectIsInvalid<typename TestFixture::TestFactoryType>(
        [](auto& sut) { const_cast<const SutType&>(sut).expect("hypnotoad ate the spagetti monster"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_RValueCase)
{
    ExpectDoesCallTerminateWhenObjectIsInvalid<typename TestFixture::TestFactoryType>(
        [](auto& sut) { std::move(sut).expect("the spagetti monster ate hypnotoad"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_constRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ExpectDoesCallTerminateWhenObjectIsInvalid<typename TestFixture::TestFactoryType>([](auto& sut) {
        std::move(const_cast<const SutType&>(sut)).expect("all glory to the hypno noodle monster toad");
    });
}

template <bool HasValue>
struct ExpectReturnsValueWhenValid;

template <>
struct ExpectReturnsValueWhenValid<false>
{
    template <typename TestFactory, typename ExpectCall>
    static void performTest(const ExpectCall&)
    {
    }
};

template <>
struct ExpectReturnsValueWhenValid<true>
{
    template <typename TestFactory, typename ExpectCall>
    static void performTest(const ExpectCall& expectCall)
    {
        auto sut = TestFactory::createValidObject();
        EXPECT_THAT(expectCall(sut), Eq(TestFactory::usedTestValue));
    }
};

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_LValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ExpectReturnsValueWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut) { return sut.expect("Earl grey with a toad flavor."); });
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ExpectReturnsValueWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut) { return const_cast<const SutType&>(sut).expect("Some cookies with flies."); });
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_RValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ExpectReturnsValueWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>(
        [](auto& sut) { return std::move(sut).expect("Sauce hollandaise with strawberries"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ExpectReturnsValueWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>([](auto& sut) {
        return std::move(const_cast<const SutType&>(sut))
            .expect("Those are the ingredients for a perfect breakfast for hypnotoad.");
    });
}
} // namespace
