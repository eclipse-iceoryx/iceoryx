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

#include "test_cxx_functional_interface_common.hpp"

namespace test_cxx_functional_interface
{
GenericValueError::GenericValueError(const value_t value, const error_t error) noexcept
    : m_value{value}
    , m_error{error}
{
}

GenericValueError::operator bool() const noexcept
{
    return m_value != 0;
}

GenericValueError::value_t& GenericValueError::value() & noexcept
{
    return m_value;
}

const GenericValueError::value_t& GenericValueError::value() const& noexcept
{
    return m_value;
}

GenericValueError::value_t&& GenericValueError::value() && noexcept
{
    return std::move(m_value);
}

const GenericValueError::value_t&& GenericValueError::value() const&& noexcept
{
    return std::move(m_value);
}

GenericValueError::error_t& GenericValueError::get_error() & noexcept
{
    return m_error;
}

const GenericValueError::error_t& GenericValueError::get_error() const& noexcept
{
    return m_error;
}

GenericValueError::error_t&& GenericValueError::get_error() && noexcept
{
    return std::move(m_error);
}

const GenericValueError::error_t&& GenericValueError::get_error() const&& noexcept
{
    return std::move(m_error);
}

GenericPlain::GenericPlain(const int value, const int)
    : m_isValid{value != INVALID_VALUE}
{
}

GenericPlain::operator bool() const noexcept
{
    return m_isValid;
}


} // namespace test_cxx_functional_interface
