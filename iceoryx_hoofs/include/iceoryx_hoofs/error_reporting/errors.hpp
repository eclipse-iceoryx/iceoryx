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

#ifndef IOX_HOOFS_ERROR_REPORTING_ERRORS_HPP
#define IOX_HOOFS_ERROR_REPORTING_ERRORS_HPP

#include <utility>

#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace iox
{
namespace er
{

static constexpr const char* UNKNOWN_MODULE_NAME = "unknown module";
static constexpr const char* UNKNOWN_ERROR_NAME = "unknown error";

// We expect an error to have the following interface
// 1. ErrorCode code() const
// 2. ModuleId module() const

// By default, there are only error codes and violations.
// Custom errors can be added but must satisfy the minimal interface.

class Violation
{
  public:
    explicit Violation(ErrorCode code)
        : m_code(code)
    {
    }

    Violation(ErrorCode code, ModuleId module)
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

    bool operator==(const Violation& rhs) const
    {
        return m_code == rhs.m_code && m_module == rhs.m_module;
    }

    bool operator!=(const Violation& rhs) const
    {
        return !(*this == rhs);
    }

    static Violation createPreconditionViolation()
    {
        return Violation(ErrorCode(ErrorCode::PRECONDITION_VIOLATION));
    }

    static Violation createAssumptionViolation()
    {
        return Violation(ErrorCode(ErrorCode::ASSUMPTION_VIOLATION));
    }

  private:
    ErrorCode m_code;
    ModuleId m_module{ModuleId::ANY};
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

template <class Error>
inline const char* toModuleName(const Error&)
{
    return UNKNOWN_MODULE_NAME;
}

template <class Error>
inline const char* toErrorName(const Error&)
{
    return UNKNOWN_ERROR_NAME;
}

} // namespace er
} // namespace iox

#endif
