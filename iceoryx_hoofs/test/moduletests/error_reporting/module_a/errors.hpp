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

#ifndef IOX_HOOFS_MODULETESTS_ERROR_REPORTING_MODULE_A_ERRORS_HPP
#define IOX_HOOFS_MODULETESTS_ERROR_REPORTING_MODULE_A_ERRORS_HPP

#include "iceoryx_hoofs/error_reporting/errors.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

namespace module_a
{
namespace errors
{

using ErrorCode = iox::err::ErrorCode;
using ModuleId = iox::err::ModuleId;

constexpr ModuleId MODULE_ID{666};

enum class Code : ErrorCode::type
{
    Unknown = 42,
    OutOfMemory = 73,
    OutOfBounds = 21
};

class Error
{
  public:
    constexpr explicit Error(Code code = Code::Unknown)
        : m_code(static_cast<ErrorCode::type>(code))
    {
    }

    static constexpr ModuleId module()
    {
        return MODULE_ID;
    }

    ErrorCode code() const
    {
        return static_cast<ErrorCode>(m_code);
    }

  protected:
    ErrorCode m_code;
};

} // namespace errors
} // namespace module_a

namespace iox
{
namespace err
{

// This definition must exist in this namespace for overload resolution.
// Each module must use a unqiue error enum, e.g. by namespace.
inline module_a::errors::Error toError(module_a::errors::Code code)
{
    return module_a::errors::Error(code);
}

// Any error code of this enum has the same module id.
inline ModuleId toModule(module_a::errors::Code)
{
    return module_a::errors::MODULE_ID;
}

} // namespace err
} // namespace iox

#endif
