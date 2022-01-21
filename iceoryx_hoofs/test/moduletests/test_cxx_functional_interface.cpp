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

#include "iceoryx_hoofs/cxx/functional_interface.hpp"
#include "iceoryx_hoofs/cxx/generic_raii.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"
#include "test.hpp"

#include <memory>

namespace
{
using namespace ::testing;

template <typename T>
class FunctionalInterface_test : public Test
{
  public:
    using TestControllerType = T;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

struct GenericValueError : public iox::cxx::FunctionalInterface<GenericValueError>
{
    using value_t = int;
    using error_t = int;

    GenericValueError(const value_t value, const error_t error)
        : m_value{value}
        , m_error{error}
    {
    }

    explicit operator bool() const noexcept
    {
        return m_value != 0;
    }

    value_t value() const noexcept
    {
        return m_value;
    }

    error_t get_error() const noexcept
    {
        return m_error;
    }

    value_t m_value = 0;
    error_t m_error = 0;
};

struct GenericPlain : public iox::cxx::FunctionalInterface<GenericPlain>
{
    explicit GenericPlain(const bool isValid)
        : m_isValid{isValid}
    {
    }

    operator bool() const noexcept
    {
        return m_isValid;
    }

    bool m_isValid = false;
};

struct GenericValueErrorTest
{
    using Type = GenericValueError;

    static Type CreateValidObject() noexcept
    {
        return GenericValueError(5, 6);
    }

    static Type CreateInvalidObject() noexcept
    {
        return GenericValueError(0, 0);
    }
};


using Implementations = Types<GenericValueErrorTest>;

TYPED_TEST_SUITE(FunctionalInterface_test, Implementations);

template <typename ControllerType, typename ExpectCall>
void ExpectDoesNotCallTerminateWhenObjectIsValid(const ExpectCall& expectCall)
{
    bool wasErrorHandlerCalled = false;
    auto sut = ControllerType::CreateValidObject();
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        expectCall(sut);
    }

    EXPECT_FALSE(wasErrorHandlerCalled);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_LValueCase)
{
    ExpectDoesNotCallTerminateWhenObjectIsValid<typename TestFixture::TestControllerType>(
        [](auto& sut) { sut.expect("a seal on the head is better then a roof on a pidgin"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestControllerType::Type;
    ExpectDoesNotCallTerminateWhenObjectIsValid<typename TestFixture::TestControllerType>([](auto& sut) {
        const_cast<const SutType&>(sut).expect(
            "hypnotoad eats unicorns for breakfast - just kidding, hypnotoad would never harm another being");
    });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_RValueCase)
{
    ExpectDoesNotCallTerminateWhenObjectIsValid<typename TestFixture::TestControllerType>(
        [](auto& sut) { std::move(sut).expect("hypnotoad is a friend of david hasselhof"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesNotCallTerminateWhenObjectIsValid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestControllerType::Type;
    ExpectDoesNotCallTerminateWhenObjectIsValid<typename TestFixture::TestControllerType>([](auto& sut) {
        std::move(const_cast<const SutType&>(sut)).expect("hypnotoads favorite animal is the leaf sheep");
    });
}

template <typename ControllerType, typename ExpectCall>
void ExpectDoesCallTerminateWhenObjectIsInvalid(const ExpectCall& expectCall)
{
    bool wasErrorHandlerCalled = true;
    auto sut = ControllerType::CreateInvalidObject();
    {
        auto handle =
            iox::ErrorHandler::setTemporaryErrorHandler([&](auto, auto, auto) { wasErrorHandlerCalled = true; });
        expectCall(sut);
    }

    EXPECT_TRUE(wasErrorHandlerCalled);
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_LValueCase)
{
    ExpectDoesCallTerminateWhenObjectIsInvalid<typename TestFixture::TestControllerType>(
        [](auto& sut) { sut.expect("the chocolate rations will be increased soon"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_constLValueCase)
{
    using SutType = typename TestFixture::TestControllerType::Type;
    ExpectDoesCallTerminateWhenObjectIsInvalid<typename TestFixture::TestControllerType>(
        [](auto& sut) { const_cast<const SutType&>(sut).expect("the chocolate rations will be increased soon"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_RValueCase)
{
    ExpectDoesCallTerminateWhenObjectIsInvalid<typename TestFixture::TestControllerType>(
        [](auto& sut) { std::move(sut).expect("the chocolate rations will be increased soon"); });
}

TYPED_TEST(FunctionalInterface_test, ExpectDoesCallTerminateWhenObjectIsInvalid_constRValueCase)
{
    using SutType = typename TestFixture::TestControllerType::Type;
    ExpectDoesCallTerminateWhenObjectIsInvalid<typename TestFixture::TestControllerType>([](auto& sut) {
        std::move(const_cast<const SutType&>(sut)).expect("the chocolate rations will be increased soon");
    });
}
} // namespace
