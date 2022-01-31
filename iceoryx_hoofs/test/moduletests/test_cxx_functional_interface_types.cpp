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

namespace test_cxx_functional_interface
{
GenericValueErrorFactory::value_t GenericValueErrorFactory::usedTestValue = 1;
GenericValueErrorFactory::value_t GenericValueErrorFactory::anotherTestValue = 2;
GenericValueErrorFactory::error_t GenericValueErrorFactory::usedErrorValue = 3;
GenericValueErrorFactory::error_t GenericValueErrorFactory::anotherErrorValue = 4;

void GenericValueErrorFactory::configureNextTestCase() noexcept
{
    usedTestValue += 23;
    anotherTestValue += 23;
    usedErrorValue += 23;
    anotherErrorValue += 23;
}

GenericValueErrorFactory::Type GenericValueErrorFactory::createValidObject() noexcept
{
    return GenericValueError(usedTestValue, 0);
}

GenericValueErrorFactory::Type GenericValueErrorFactory::createInvalidObject() noexcept
{
    return GenericValueError(0, usedErrorValue);
}

void GenericPlainFactory::configureNextTestCase() noexcept
{
}

GenericPlain GenericPlainFactory::createValidObject() noexcept
{
    return GenericPlain(5, 6);
}

GenericPlain GenericPlainFactory::createInvalidObject() noexcept
{
    return GenericPlain(0, 0);
}
} // namespace test_cxx_functional_interface
