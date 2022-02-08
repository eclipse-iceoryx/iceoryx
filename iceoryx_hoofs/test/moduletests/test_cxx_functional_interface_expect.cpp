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
#include "iceoryx_hoofs/testing/mocks/error_handler_mock.hpp"

#include "test_cxx_functional_interface_types.hpp"

namespace
{
using namespace test_cxx_functional_interface;
using namespace ::testing;

#define IOX_TEST_FUNCTIONAL_INTERFACE(TestName, variationPoint)                                                        \
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
        auto handle = iox::ErrorHandlerMock::setTemporaryErrorHandler<iox::Error>(
            [&](auto, auto) { wasErrorHandlerCalled = true; });
        callExpect(sut);
    }

    EXPECT_FALSE(wasErrorHandlerCalled);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_LValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "21d71373-39ae-499f-856e-96014f1c2c25");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectDoesNotCallTerminateWhenObjectIsValid, sut);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_ConstLValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "252fe5e0-eb3e-4e9b-a03d-36c4e2344d39");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectDoesNotCallTerminateWhenObjectIsValid, const_cast<const SutType&>(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_RValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "1739aa02-568b-4f6f-89d6-423ef6ab6bdc");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectDoesNotCallTerminateWhenObjectIsValid, std::move(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_ConstRValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "86bd8ee1-7b05-4e64-88c6-b4359f87d346");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectDoesNotCallTerminateWhenObjectIsValid,
                                  std::move(const_cast<const SutType&>(sut)));
}

template <typename FactoryType, typename SutType, typename ExpectCall>
void ExpectDoesCallTerminateWhenObjectIsInvalid(const ExpectCall& callExpect)
{
    bool wasErrorHandlerCalled = true;
    SutType sut = FactoryType::createInvalidObject();
    {
        auto handle = iox::ErrorHandlerMock::setTemporaryErrorHandler<iox::Error>(
            [&](auto, auto) { wasErrorHandlerCalled = true; });
        callExpect(sut);
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_LValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "bcaf74b0-070e-4ca9-a3c9-e41c331420e6");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectDoesCallTerminateWhenObjectIsInvalid, sut);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_constLValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "52e66941-416a-45d6-bb33-e6a1c3824692");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectDoesCallTerminateWhenObjectIsInvalid, const_cast<const SutType&>(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_RValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "6e8e9982-bd9f-4aa7-8756-b21c288a658d");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectDoesCallTerminateWhenObjectIsInvalid, std::move(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_constRValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "cbdf0b40-d4bb-41a6-b811-dcafc96c86de");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectDoesCallTerminateWhenObjectIsInvalid,
                                  std::move(const_cast<const SutType&>(sut)));
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

#undef IOX_TEST_FUNCTIONAL_INTERFACE
#define IOX_TEST_FUNCTIONAL_INTERFACE(TestName, variationPoint)                                                        \
    using SutType = typename TestFixture::TestFactoryType::Type;                                                       \
    constexpr bool HAS_VALUE_METHOD = iox::cxx::internal::HasValueMethod<SutType>::value;                              \
    TestName<HAS_VALUE_METHOD>::template performTest<typename TestFixture::TestFactoryType>([](auto& sut) {            \
        return variationPoint.expect(                                                                                  \
            "hypnotoad eats unicorns for breakfast - just kidding, hypnotoad would never harm another being");         \
    })


TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_LValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "ab3c6a55-f218-4750-a6b6-e40d946d5b7e");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectReturnsValueWhenValid, sut);
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_ConstLValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "b699d117-ba1d-4806-86b3-0a92dc255cbb");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectReturnsValueWhenValid, const_cast<const SutType&>(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_RValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "0fdd90d0-30b1-432f-97f5-2d98125051fe");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectReturnsValueWhenValid, std::move(sut));
}

TYPED_TEST(FunctionalInterface_test, ExpectReturnsValueWhenValid_ConstRValueCase)
{
    ::testing::Test::RecordProperty("TEST_ID", "49e22cde-eae3-4fb5-b078-7a5d53916171");
    IOX_TEST_FUNCTIONAL_INTERFACE(ExpectReturnsValueWhenValid, std::move(const_cast<const SutType&>(sut)));
}

#undef IOX_TEST_FUNCTIONAL_INTERFACE
} // namespace
