// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_BINDING_C_ERROR_HANDLING_ERROR_HANDLING_HPP
#define IOX_BINDING_C_BINDING_ERROR_HANDLING_ERROR_HANDLING_HPP

#include "iceoryx_hoofs/error_handling/error_handling.hpp"

namespace iox
{
// clang-format off
#define C_BINDING_ERRORS(error) \
    error(BINDING_C__UNDEFINED_STATE_IN_IOX_QUEUE_FULL_POLICY) \
    error(BINDING_C__UNDEFINED_STATE_IN_IOX_SUBSCRIBER_TOO_SLOW_POLICY) \
    error(BINDING_C__PUBLISHER_OPTIONS_NOT_INITIALIZED) \
    error(BINDING_C__SUBSCRIBER_OPTIONS_NOT_INITIALIZED) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_EVENT_VALUE) \
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_SUBSCRIBER_STATE_VALUE)

// clang-format on

// DO NOT TOUCH START, you can doodle around with the lines above!!!
#define CREATE_ICEORYX_ERROR_ENUM(name) k##name,
#define CREATE_ICEORYX_ERROR_STRING(name) #name,

enum class CBindingError : uint32_t
{
    kNO_ERROR = 2000,
    C_BINDING_ERRORS(CREATE_ICEORYX_ERROR_ENUM)
};

/// @brief Convenience stream operator to easily use the Error enum with std::ostream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to `stream` which was provided as input parameter
std::ostream& operator<<(std::ostream& stream, CBindingError value) noexcept;

template <>
inline void ErrorHandler<CBindingError>::defaultHandler(const CBindingError error,
                                                        const std::function<void()>& errorCallBack,
                                                        const ErrorLevel level) noexcept
{
    if (errorCallBack)
    {
        errorCallBack();
    }
    else
    {
        std::stringstream ss;
        ss << "iceoryx_binding_c error! " << error;

        reactOnErrorLevel(level, ss.str().c_str());
    }
}
} // namespace iox
#endif // IOX_BINDING_C_ERROR_HANDLING_ERROR_HANDLING_HPP
