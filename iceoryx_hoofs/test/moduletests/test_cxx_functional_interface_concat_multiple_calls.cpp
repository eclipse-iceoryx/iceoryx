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

template <bool HasValue, bool HasError>
struct AndThenOrElseConcatenatedWorksWhenInvalid;

template <>
struct AndThenOrElseConcatenatedWorksWhenInvalid<false, false>
{
    template <typename TestFactory, typename SutCall>
    static void performTest(const SutCall& sutCall)
    {
        auto sut = TestFactory::createInvalidObject();
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
        auto sut = TestFactory::createInvalidObject();
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
        auto sut = TestFactory::createInvalidObject();
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
        auto sut = TestFactory::createInvalidObject();
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
        auto sut = TestFactory::createValidObject();
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
        auto sut = TestFactory::createValidObject();
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
        auto sut = TestFactory::createValidObject();
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
        auto sut = TestFactory::createValidObject();
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
