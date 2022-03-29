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

#include "iceoryx_binding_c/error_handling/error_handling.hpp"

namespace iox
{
const char* C_BINDING_ERROR_NAMES[] = {C_BINDING_ERRORS(CREATE_ICEORYX_ERROR_STRING)};

const char* asStringLiteral(const CBindingError error) noexcept
{
    return C_BINDING_ERROR_NAMES[errorToStringIndex(error)];
}
} // namespace iox
