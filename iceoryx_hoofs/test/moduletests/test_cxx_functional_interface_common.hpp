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

#ifndef IOX_HOOFS_MODULETESTS_TEST_CXX_FUNCTIONAL_INTERFACE_COMMON_HPP
#define IOX_HOOFS_MODULETESTS_TEST_CXX_FUNCTIONAL_INTERFACE_COMMON_HPP

#include "iceoryx_hoofs/cxx/functional_interface.hpp"
#include "test.hpp"

namespace test_cxx_functional_interface
{
template <typename T>
class FunctionalInterface_test : public testing::Test
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
    GenericValueError(const int value, const int error) noexcept;

    explicit operator bool() const noexcept;

    int& value() & noexcept;
    const int& value() const& noexcept;
    int&& value() && noexcept;
    const int&& value() const&& noexcept;

    int& get_error() & noexcept;
    const int& get_error() const& noexcept;
    int&& get_error() && noexcept;
    const int&& get_error() const&& noexcept;

    int m_value = 0;
    int m_error = 0;
};

struct GenericPlain : public iox::cxx::FunctionalInterface<GenericPlain, void, void>
{
    explicit GenericPlain(const int value, const int);
    operator bool() const noexcept;

    bool m_isValid = false;
};


} // namespace test_cxx_functional_interface

#endif
