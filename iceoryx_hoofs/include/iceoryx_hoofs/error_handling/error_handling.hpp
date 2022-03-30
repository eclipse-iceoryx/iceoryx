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
#ifndef IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLING_HPP
#define IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLING_HPP

#include "iceoryx_hoofs/error_handling/error_handler.hpp"

namespace iox
{
// clang-format off
#define HOOFS_ERRORS(error) \
    error(EXPECTS_ENSURES_FAILED)

    // EXPECTS_ENSURES_FAILED is used as a temporary solution to make Expects/Ensures testable

// clang-format on

// DO NOT TOUCH THE ENUM, you can doodle around with the lines above!!!

enum class HoofsError : uint32_t
{
    NO_ERROR = HOOFS_MODULE_IDENTIFIER << ERROR_ENUM_OFFSET_IN_BITS,
    HOOFS_ERRORS(CREATE_ICEORYX_ERROR_ENUM)
};

const char* asStringLiteral(const HoofsError error) noexcept;

} // namespace iox

#endif // IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLING_HPP
