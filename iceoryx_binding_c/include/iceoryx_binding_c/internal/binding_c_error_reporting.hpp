// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_BINDING_C_BINDING_C_ERROR_REPORTING_HPP
#define IOX_BINDING_C_BINDING_C_ERROR_REPORTING_HPP

// Each module (= some unit with its own errors) must provide the following.

// 1. Define the errors of the module -> see below

// 2. Include the custom reporting implementation
#include "iox/error_reporting/custom/error_reporting.hpp"

// 3. Include the error reporting macro API
#include "iox/error_reporting/macros.hpp"

// additional includes
#include "iox/error_reporting/types.hpp"

/// @todo iox-#1032 Remove once C_BINDING_MODULE_IDENTIFIER is moved to 'ModuleId' in 'error_reporting/types.hpp'
#include "iceoryx_hoofs/error_handling/error_handler.hpp"

namespace iox
{

// clang-format off
#define C_BINDING_ERRORS(error) \
    error(BINDING_C__UNDEFINED_STATE_IN_IOX_QUEUE_FULL_POLICY) \
    error(BINDING_C__UNDEFINED_STATE_IN_IOX_CONSUMER_TOO_SLOW_POLICY) \
    error(BINDING_C__PUBLISHER_OPTIONS_NOT_INITIALIZED) \
    error(BINDING_C__SUBSCRIBER_OPTIONS_NOT_INITIALIZED) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_EVENT_VALUE) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_STATE_VALUE) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_CLIENT_EVENT_VALUE) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_CLIENT_STATE_VALUE) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVER_EVENT_VALUE) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVER_STATE_VALUE) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SERVICE_DISCOVERY_EVENT_VALUE) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_MESSAGING_PATTERN_VALUE) \
    error(DO_NOT_USE_AS_ERROR_THIS_IS_AN_INTERNAL_MARKER) // keep this always at the end of the error list

// clang-format on

// DO NOT TOUCH THE ENUM, you can doodle around with the lines above!!!

enum class CBindingError : iox::er::ErrorCode::type
{
    C_BINDING_ERRORS(CREATE_ICEORYX_ERROR_ENUM)
};

const char* asStringLiteral(const CBindingError error) noexcept;

class CBindingErrorType
{
  public:
    explicit CBindingErrorType(CBindingError code)
        : m_code(static_cast<iox::er::ErrorCode::type>(code))
    {
    }

    static constexpr iox::er::ModuleId module()
    {
        return MODULE_ID;
    }

    iox::er::ErrorCode code() const
    {
        return m_code;
    }

    const char* name() const
    {
        return asStringLiteral(static_cast<CBindingError>(m_code.value));
    }

    static const char* moduleName()
    {
        return "iceoryx_binding_c";
    }

    static constexpr iox::er::ModuleId MODULE_ID{C_BINDING_MODULE_IDENTIFIER};

  protected:
    iox::er::ErrorCode m_code;
};

namespace er
{

inline CBindingErrorType toError(CBindingError code)
{
    return CBindingErrorType(code);
}

inline ModuleId toModule(CBindingError)
{
    return CBindingErrorType::MODULE_ID;
}

} // namespace er
} // namespace iox

#endif // IOX_BINDING_C_BINDING_C_ERROR_REPORTING_HPP
