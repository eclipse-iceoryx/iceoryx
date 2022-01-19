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
#include "test.hpp"

#include <memory>

namespace
{
using namespace ::testing;

template <typename T>
class FunctionalInterface_test : public Test
{
  public:
    using TestType = T;

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

    operator bool() const noexcept
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

    static Type CreateValid() noexcept
    {
    }
};


using Implementations = Types<GenericValueError>;

TYPED_TEST_SUITE(FunctionalInterface_test, Implementations);

TYPED_TEST(FunctionalInterface_test, DefaultCTorHasValue)
{
}
} // namespace
