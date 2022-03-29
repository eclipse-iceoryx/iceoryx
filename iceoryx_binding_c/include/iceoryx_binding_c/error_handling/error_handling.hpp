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
#ifndef IOX_BINDING_C_BINDING_ERROR_HANDLING_ERROR_HANDLING_HPP
#define IOX_BINDING_C_BINDING_ERROR_HANDLING_ERROR_HANDLING_HPP

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
    error(BINDING_C__C2CPP_ENUM_TRANSLATION_INVALID_MESSAGING_PATTERN_VALUE)

// clang-format on

// DO NOT TOUCH THE ENUM, you can doodle around with the lines above!!!

enum class CBindingError : uint32_t
{
    NO_ERROR = C_BINDING_MODULE_IDENTIFIER << ERROR_ENUM_OFFSET_IN_BITS,
    C_BINDING_ERRORS(CREATE_ICEORYX_ERROR_ENUM)
};

const char* asStringLiteral(const CBindingError error) noexcept;

} // namespace iox
#endif // IOX_BINDING_C_ERROR_HANDLING_ERROR_HANDLING_HPP
