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

#ifndef IOX_HOOFS_MODULETESTS_TEST_CXX_FUNCTIONAL_INTERFACE_TYPES_HPP
#define IOX_HOOFS_MODULETESTS_TEST_CXX_FUNCTIONAL_INTERFACE_TYPES_HPP

#include "test_cxx_functional_interface_common.hpp"

namespace test_cxx_functional_interface
{
struct GenericValueErrorFactory
{
    using Type = GenericValueError;

    using value_t = int;
    using error_t = int;

    static int usedTestValue;
    static int anotherTestValue;

    static int usedErrorValue;
    static int anotherErrorValue;

    static void configureNextTestCase() noexcept;
    static GenericValueError createValidObject() noexcept;
    static GenericValueError createInvalidObject() noexcept;
};


struct GenericPlainFactory
{
    using Type = GenericPlain;

    static void configureNextTestCase() noexcept;
    static Type createValidObject() noexcept;
    static Type createInvalidObject() noexcept;
};

using FunctionalInterfaceImplementations = testing::Types<GenericValueErrorFactory, GenericPlainFactory>;
TYPED_TEST_SUITE(FunctionalInterface_test, FunctionalInterfaceImplementations);

} // namespace test_cxx_functional_interface

#endif
