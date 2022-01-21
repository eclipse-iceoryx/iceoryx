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
    using TestFactoryType = T;

    void SetUp() override
    {
        T::configureNextTestCase();
    }

    void TearDown() override
    {
    }
};

struct GenericValueError : public iox::cxx::FunctionalInterface<GenericValueError, int, int>
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

    value_t& value() & noexcept
    {
        return m_value;
    }

    const value_t& value() const& noexcept
    {
        return m_value;
    }

    value_t&& value() && noexcept
    {
        return std::move(m_value);
    }

    const value_t&& value() const&& noexcept
    {
        return std::move(m_value);
    }


    error_t& get_error() & noexcept
    {
        return m_error;
    }

    const error_t& get_error() const& noexcept
    {
        return m_error;
    }

    error_t&& get_error() && noexcept
    {
        return std::move(m_error);
    }

    const error_t&& get_error() const&& noexcept
    {
        return std::move(m_error);
    }

    value_t m_value = 0;
    error_t m_error = 0;
};

struct GenericPlain : public iox::cxx::FunctionalInterface<GenericPlain, void, void>
{
    explicit GenericPlain(const int value, const int)
        : m_isValid{value != 0}
    {
    }

    operator bool() const noexcept
    {
        return m_isValid;
    }

    bool m_isValid = false;
};

struct GenericValueErrorFactory
{
    using Type = GenericValueError;

    static int usedTestValue;
    static int anotherTestValue;

    static int usedErrorValue;
    static int anotherErrorValue;

    static void configureNextTestCase() noexcept
    {
        usedTestValue += 23;
        anotherTestValue += 23;
        usedErrorValue += 23;
        anotherErrorValue += 23;
    }

    static Type CreateValidObject() noexcept
    {
        return Type(usedTestValue, 0);
    }

    static Type CreateInvalidObject() noexcept
    {
        return Type(0, usedErrorValue);
    }
};

int GenericValueErrorFactory::usedTestValue = 1;
int GenericValueErrorFactory::anotherTestValue = 2;
int GenericValueErrorFactory::usedErrorValue = 3;
int GenericValueErrorFactory::anotherErrorValue = 4;

struct GenericPlainFactory
{
    using Type = GenericPlain;

    static void configureNextTestCase() noexcept
    {
    }

    static Type CreateValidObject() noexcept
    {
        return Type(5, 6);
    }

    static Type CreateInvalidObject() noexcept
    {
        return Type(0, 0);
    }
};

using Implementations = Types<GenericValueErrorFactory, GenericPlainFactory>;

TYPED_TEST_SUITE(FunctionalInterface_test, Implementations);

template <typename FactoryType, typename ExpectCall>
void ExpectDoesNotCallTerminateWhenObjectIsValid(const ExpectCall& expectCall)
{
    bool wasErrorHandlerCalled = false;
    auto sut = FactoryType::CreateValidObject();
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
    auto sut = FactoryType::CreateInvalidObject();
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
        auto sut = TestFactory::CreateValidObject();
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

template <bool HasValue>
struct ValueOrReturnsValueWhenValid;

template <>
struct ValueOrReturnsValueWhenValid<false>
{
    template <typename TestFactory>
    static void performTest()
    {
    }
};

template <>
struct ValueOrReturnsValueWhenValid<true>
{
    template <typename TestFactory>
    static void performTest()
    {
        auto sut = TestFactory::CreateValidObject();
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
struct ValueOrReturnsArgumentWhenInalid<false>
{
    template <typename TestFactory>
    static void performTest()
    {
    }
};

template <>
struct ValueOrReturnsArgumentWhenInalid<true>
{
    template <typename TestFactory>
    static void performTest()
    {
        auto sut = TestFactory::CreateInvalidObject();
        EXPECT_THAT(sut.value_or(TestFactory::anotherTestValue), Eq(TestFactory::anotherTestValue));
    }
};

TYPED_TEST(FunctionalInterface_test, ValueOrReturnsArgumentWhenInalid)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    ValueOrReturnsArgumentWhenInalid<iox::cxx::internal::HasValueMethod<SutType>::value>::template performTest<
        typename TestFixture::TestFactoryType>();
}

template <bool HasValue>
struct AndThenIsCalledCorrectlyWhenValid;

template <>
struct AndThenIsCalledCorrectlyWhenValid<false>
{
    template <typename TestFactory, typename AndThenCall>
    static void performTest(const AndThenCall& andThenCall)
    {
        auto sut = TestFactory::CreateValidObject();
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
        auto sut = TestFactory::CreateValidObject();
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
        auto sut = TestFactory::CreateInvalidObject();
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
        auto sut = TestFactory::CreateInvalidObject();
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

template <bool HasError>
struct OrElseIsCalledCorrectlyWhenInvalid;

template <>
struct OrElseIsCalledCorrectlyWhenInvalid<false>
{
    template <typename TestFactory, typename OrElseCall>
    static void performTest(const OrElseCall& orElseCall)
    {
        auto sut = TestFactory::CreateInvalidObject();
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
        auto sut = TestFactory::CreateInvalidObject();
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
        auto sut = TestFactory::CreateValidObject();
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
        auto sut = TestFactory::CreateValidObject();
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

template <bool HasValue, bool HasError>
struct AndThenOrElseConcatenatedWorksWhenInvalid;

template <>
struct AndThenOrElseConcatenatedWorksWhenInvalid<false, false>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::CreateInvalidObject();
        bool wasAndThenCalled = false;
        bool wasOrElseCalled = false;
        sutCall(
            sut, [&] { wasAndThenCalled = true; }, [&] { wasOrElseCalled = true; });

        EXPECT_FALSE(wasAndThenCalled);
        EXPECT_TRUE(wasOrElseCalled);
    }
};

template <>
struct AndThenOrElseConcatenatedWorksWhenInvalid<true, false>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::CreateInvalidObject();
        bool wasAndThenCalled = false;
        bool wasOrElseCalled = false;
        sutCall(
            sut, [&](auto&) { wasAndThenCalled = true; }, [&] { wasOrElseCalled = true; });

        EXPECT_FALSE(wasAndThenCalled);
        EXPECT_TRUE(wasOrElseCalled);
    }
};

template <>
struct AndThenOrElseConcatenatedWorksWhenInvalid<false, true>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::CreateInvalidObject();
        bool wasAndThenCalled = false;
        bool wasOrElseCalled = false;
        sutCall(
            sut,
            [&] { wasAndThenCalled = true; },
            [&](auto& error) {
                wasOrElseCalled = true;
                EXPECT_THAT(error, Eq(TestFactory::usedErrorValue));
            });

        EXPECT_FALSE(wasAndThenCalled);
        EXPECT_TRUE(wasOrElseCalled);
    }
};

template <>
struct AndThenOrElseConcatenatedWorksWhenInvalid<true, true>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::CreateInvalidObject();
        bool wasAndThenCalled = false;
        bool wasOrElseCalled = false;

        sutCall(
            sut,
            [&](auto&) { wasAndThenCalled = true; },
            [&](auto& error) {
                wasOrElseCalled = true;
                EXPECT_THAT(error, Eq(TestFactory::usedErrorValue));
            });
        EXPECT_FALSE(wasAndThenCalled);
        EXPECT_TRUE(wasOrElseCalled);
    }
};

TYPED_TEST(FunctionalInterface_test, AndThenOrElseConcatenatedWorksWhenInvalid_LValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenOrElseConcatenatedWorksWhenInvalid<iox::cxx::internal::HasValueMethod<SutType>::value,
                                              iox::cxx::internal::HasGetErrorMethod<SutType>::value>::
        template performTest<typename TestFixture::TestFactoryType>(
            [](auto& sut, auto andThenCallback, auto orElseCallback) {
                sut.and_then(andThenCallback).or_else(orElseCallback);
            });
}

TYPED_TEST(FunctionalInterface_test, AndThenOrElseConcatenatedWorksWhenInvalid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenOrElseConcatenatedWorksWhenInvalid<iox::cxx::internal::HasValueMethod<SutType>::value,
                                              iox::cxx::internal::HasGetErrorMethod<SutType>::value>::
        template performTest<typename TestFixture::TestFactoryType>(
            [](auto& sut, auto andThenCallback, auto orElseCallback) {
                const_cast<const SutType&>(sut).and_then(andThenCallback).or_else(orElseCallback);
            });
}

TYPED_TEST(FunctionalInterface_test, AndThenOrElseConcatenatedWorksWhenInvalid_RValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenOrElseConcatenatedWorksWhenInvalid<iox::cxx::internal::HasValueMethod<SutType>::value,
                                              iox::cxx::internal::HasGetErrorMethod<SutType>::value>::
        template performTest<typename TestFixture::TestFactoryType>(
            [](auto& sut, auto andThenCallback, auto orElseCallback) {
                std::move(sut).and_then(andThenCallback).or_else(orElseCallback);
            });
}

TYPED_TEST(FunctionalInterface_test, AndThenOrElseConcatenatedWorksWhenInvalid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    AndThenOrElseConcatenatedWorksWhenInvalid<iox::cxx::internal::HasValueMethod<SutType>::value,
                                              iox::cxx::internal::HasGetErrorMethod<SutType>::value>::
        template performTest<typename TestFixture::TestFactoryType>(
            [](auto& sut, auto andThenCallback, auto orElseCallback) {
                std::move(const_cast<const SutType&>(sut)).and_then(andThenCallback).or_else(orElseCallback);
            });
}

template <bool HasValue, bool HasError>
struct OrElseAndThenConcatenatedWorkWhenValid;

template <>
struct OrElseAndThenConcatenatedWorkWhenValid<false, false>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::CreateValidObject();
        bool wasAndThenCalled = false;
        bool wasOrElseCalled = false;
        sutCall(
            sut, [&] { wasAndThenCalled = true; }, [&] { wasOrElseCalled = true; });

        EXPECT_TRUE(wasAndThenCalled);
        EXPECT_FALSE(wasOrElseCalled);
    }
};

template <>
struct OrElseAndThenConcatenatedWorkWhenValid<true, false>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::CreateValidObject();
        bool wasAndThenCalled = false;
        bool wasOrElseCalled = false;
        sutCall(
            sut,
            [&](auto& value) {
                wasAndThenCalled = true;
                EXPECT_THAT(value, Eq(TestFactory::usedTestValue));
            },
            [&] { wasOrElseCalled = true; });

        EXPECT_TRUE(wasAndThenCalled);
        EXPECT_FALSE(wasOrElseCalled);
    }
};

template <>
struct OrElseAndThenConcatenatedWorkWhenValid<false, true>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::CreateValidObject();
        bool wasAndThenCalled = false;
        bool wasOrElseCalled = false;
        sutCall(
            sut, [&] { wasAndThenCalled = true; }, [&](auto&) { wasOrElseCalled = true; });

        EXPECT_TRUE(wasAndThenCalled);
        EXPECT_FALSE(wasOrElseCalled);
    }
};

template <>
struct OrElseAndThenConcatenatedWorkWhenValid<true, true>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::CreateValidObject();
        bool wasAndThenCalled = false;
        bool wasOrElseCalled = false;

        sutCall(
            sut,
            [&](auto& value) {
                wasAndThenCalled = true;
                EXPECT_THAT(value, Eq(TestFactory::usedTestValue));
            },
            [&](auto&) { wasOrElseCalled = true; });
        EXPECT_TRUE(wasAndThenCalled);
        EXPECT_FALSE(wasOrElseCalled);
    }
};

TYPED_TEST(FunctionalInterface_test, OrElseAndThenConcatenatedWorkWhenValid_LValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseAndThenConcatenatedWorkWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value,
                                           iox::cxx::internal::HasGetErrorMethod<SutType>::value>::
        template performTest<typename TestFixture::TestFactoryType>(
            [](auto& sut, auto andThenCallback, auto orElseCallback) {
                sut.or_else(orElseCallback).and_then(andThenCallback);
            });
}

TYPED_TEST(FunctionalInterface_test, OrElseAndThenConcatenatedWorkWhenValid_ConstLValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseAndThenConcatenatedWorkWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value,
                                           iox::cxx::internal::HasGetErrorMethod<SutType>::value>::
        template performTest<typename TestFixture::TestFactoryType>(
            [](auto& sut, auto andThenCallback, auto orElseCallback) {
                const_cast<const SutType&>(sut).or_else(orElseCallback).and_then(andThenCallback);
            });
}

TYPED_TEST(FunctionalInterface_test, OrElseAndThenConcatenatedWorkWhenValid_RValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseAndThenConcatenatedWorkWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value,
                                           iox::cxx::internal::HasGetErrorMethod<SutType>::value>::
        template performTest<typename TestFixture::TestFactoryType>(
            [](auto& sut, auto andThenCallback, auto orElseCallback) {
                std::move(sut).or_else(orElseCallback).and_then(andThenCallback);
            });
}

TYPED_TEST(FunctionalInterface_test, OrElseAndThenConcatenatedWorkWhenValid_ConstRValueCase)
{
    using SutType = typename TestFixture::TestFactoryType::Type;
    OrElseAndThenConcatenatedWorkWhenValid<iox::cxx::internal::HasValueMethod<SutType>::value,
                                           iox::cxx::internal::HasGetErrorMethod<SutType>::value>::
        template performTest<typename TestFixture::TestFactoryType>(
            [](auto& sut, auto andThenCallback, auto orElseCallback) {
                std::move(const_cast<const SutType&>(sut)).or_else(orElseCallback).and_then(andThenCallback);
            });
}
} // namespace
