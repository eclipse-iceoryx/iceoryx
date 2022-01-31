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

#define IOX_TEST(TestName, variationPoint)                                                                             \
    using SutType = typename TestFixture::TestFactoryType::Type;                                                       \
    TestName<typename TestFixture::TestFactoryType, SutType>([](auto& sut) {                                           \
        variationPoint.expect(                                                                                         \
            "hypnotoad eats unicorns for breakfast - just kidding, hypnotoad would never harm another being");         \
    })

template <typename FactoryType, typename SutType, typename ExpectCall>
void ExpectDoesNotCallTerminateWhenObjectIsValid(const ExpectCall& callExpect)
{
    bool wasErrorHandlerCalled = false;
    SutType sut = FactoryType::createValidObject();
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        callExpect(sut);
    }

    EXPECT_FALSE(wasErrorHandlerCalled);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_LValueCase)
{
    IOX_TEST(ExpectDoesNotCallTerminateWhenObjectIsValid, sut);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_ConstLValueCase)
{
    IOX_TEST(ExpectDoesNotCallTerminateWhenObjectIsValid, const_cast<const SutType&>(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_RValueCase)
{
    IOX_TEST(ExpectDoesNotCallTerminateWhenObjectIsValid, std::move(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_ConstRValueCase)
{
    IOX_TEST(ExpectDoesNotCallTerminateWhenObjectIsValid, std::move(const_cast<const SutType&>(sut)));
}

template <typename FactoryType, typename SutType, typename ExpectCall>
void ExpectDoesCallTerminateWhenObjectIsInvalid(const ExpectCall& callExpect)
{
    bool wasErrorHandlerCalled = true;
    SutType sut = FactoryType::createInvalidObject();
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        callExpect(sut);
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_LValueCase)
{
    IOX_TEST(ExpectDoesCallTerminateWhenObjectIsInvalid, sut);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_constLValueCase)
{
    IOX_TEST(ExpectDoesCallTerminateWhenObjectIsInvalid, const_cast<const SutType&>(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_RValueCase)
{
    IOX_TEST(ExpectDoesCallTerminateWhenObjectIsInvalid, std::move(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_constRValueCase)
{
    IOX_TEST(ExpectDoesCallTerminateWhenObjectIsInvalid, std::move(const_cast<const SutType&>(sut)));
}

constexpr bool TYPE_HAS_VALUE_METHOD = true;
constexpr bool TYPE_HAS_NO_VALUE_METHOD = false;

template <bool HasValue>
struct ExpectReturnsValueWhenValid;

template <>
struct ExpectReturnsValueWhenValid<TYPE_HAS_NO_VALUE_METHOD>
{
    template <typename TestFactory, typename ExpectCall>
    static void performTest(const ExpectCall&)
    {
    }
};

template <>
struct ExpectReturnsValueWhenValid<TYPE_HAS_VALUE_METHOD>
{
    template <typename TestFactory, typename ExpectCall>
    static void performTest(const ExpectCall& callExpect)
    {
        auto sut = TestFactory::createValidObject();
        EXPECT_THAT(callExpect(sut), Eq(TestFactory::usedTestValue));
    }
};

#undef IOX_TEST
#define IOX_TEST(TestName, variationPoint)                                                                             \
    using SutType = typename TestFixture::TestFactoryType::Type;                                                       \
    constexpr bool HAS_VALUE_METHOD = iox::cxx::internal::HasValueMethod<SutType>::value;                              \
    TestName<HAS_VALUE_METHOD>::template performTest<typename TestFixture::TestFactoryType>([](auto& sut) {            \
        return variationPoint.expect(                                                                                  \
            "hypnotoad eats unicorns for breakfast - just kidding, hypnotoad would never harm another being");         \
    })


TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_LValueCase)
{
    IOX_TEST(ExpectReturnsValueWhenValid, sut);
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_ConstLValueCase)
{
    IOX_TEST(ExpectReturnsValueWhenValid, const_cast<const SutType&>(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_RValueCase)
{
    IOX_TEST(ExpectReturnsValueWhenValid, std::move(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_ConstRValueCase)
{
    IOX_TEST(ExpectReturnsValueWhenValid, std::move(const_cast<const SutType&>(sut)));
}

#undef IOX_TEST
} // namespace
