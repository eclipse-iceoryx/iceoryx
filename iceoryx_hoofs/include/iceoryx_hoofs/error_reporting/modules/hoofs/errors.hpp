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

#ifndef IOX_HOOFS_ERROR_REPORTING_MODULES_HOOFS_ERRORS_HPP
#define IOX_HOOFS_ERROR_REPORTING_MODULES_HOOFS_ERRORS_HPP

#include "iceoryx_hoofs/error_reporting/errors.hpp"
#include "iceoryx_hoofs/error_reporting/types.hpp"

/// @todo iox-#1032 Incomplete and not used yet, will be used in integration or error reporting into
/// iceoryx_hoofs. This is just a sketch/proof of concept.

namespace iox
{
namespace hoofs_errors
{

using ModuleId = iox::er::ModuleId;

enum class Code : iox::er::ErrorCode::type
{
    Unknown = 0
};

class Error
{
  public:
    explicit Error(Code code = Code::Unknown)
        : m_code(static_cast<iox::er::ErrorCode::type>(code))
    {
    }

    static constexpr ModuleId module()
    {
        return MODULE_ID;
    }

    iox::er::ErrorCode code() const
    {
        return m_code;
    }

    const char* name() const
    {
        /// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index) temporary mechanism
        return errorNames[m_code.value];
    }

    static constexpr ModuleId MODULE_ID{1};

  protected:
    iox::er::ErrorCode m_code;

    /// @todo iox-#1032 Incomplete and not used yet, use a robust compile time mechanism to define names
    /// in integration follow up
    /// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays) temporary mechanism
    static constexpr const char* errorNames[] = {"Unknown"};
};

} // namespace hoofs_errors
} // namespace iox

namespace iox
{
namespace er
{

inline hoofs_errors::Error toError(hoofs_errors::Code code)
{
    return hoofs_errors::Error(code);
}

inline ModuleId toModule(hoofs_errors::Code)
{
    return hoofs_errors::Error::MODULE_ID;
}

} // namespace er
} // namespace iox

#endif
