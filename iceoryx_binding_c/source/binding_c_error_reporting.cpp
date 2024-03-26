// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_binding_c/internal/binding_c_error_reporting.hpp"

namespace iox
{
const char* BINDING_C_ERROR_NAMES[] = {IOX_BINDING_C_ERRORS(IOX_CREATE_ERROR_STRING)};

const char* asStringLiteral(const CBindingError error) noexcept
{
    auto end = static_cast<std::underlying_type<CBindingError>::type>(
        CBindingError::DO_NOT_USE_AS_ERROR_THIS_IS_AN_INTERNAL_MARKER);
    auto index = static_cast<std::underlying_type<CBindingError>::type>(error);
    if (index >= end)
    {
        return "Unknown Error Code!";
    }
    // NOLINTJUSTIFICATION Bounds are checked and access is safe
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return BINDING_C_ERROR_NAMES[index];
}
} // namespace iox
