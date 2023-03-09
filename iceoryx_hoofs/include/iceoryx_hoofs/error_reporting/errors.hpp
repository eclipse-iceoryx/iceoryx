// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_ERROR_REPORTING_ERROR_HPP
#define IOX_HOOFS_ERROR_REPORTING_ERROR_HPP

#include <utility>

#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace iox
{
namespace err
{

// We expect an error to have the following interface
// 1. ErrorCode code() const
// 2. ModuleId module() const
// 3. const char* name() const
// The latter must return a pointer to the data segment (no dynamic memory) to be efficient.

// By default, there are only violations and error codes.

class Violation
{
  public:
    explicit Violation(ErrorCode::type code)
        : m_code(code)
    {
    }

    Violation(ErrorCode::type code, ModuleId module)
        : m_code(code)
        , m_module(module)
    {
    }

    ErrorCode code() const
    {
        return m_code;
    }

    ModuleId module() const
    {
        return m_module;
    }

    const char* name() const
    {
        return NAME;
    }

  public:
    ErrorCode m_code{ErrorCode::DEBUG_ASSERT_VIOLATION};
    ModuleId m_module{ModuleId::ANY};

    static constexpr const char* NAME = "Violation";
};

// primary template is the identity
// this can be overriden by modules to handle specific errors
template <typename ErrorLike>
auto toError(ErrorLike&& value)
{
    return std::forward<ErrorLike>(value);
}

template <class Error>
inline ErrorCode toCode(const Error& error)
{
    return error.code();
}

template <>
inline ErrorCode toCode<ErrorCode>(const ErrorCode& error)
{
    return error;
}

template <class Error>
inline ModuleId toModule(const Error& error)
{
    return error.module();
}

} // namespace err
} // namespace iox

#endif
