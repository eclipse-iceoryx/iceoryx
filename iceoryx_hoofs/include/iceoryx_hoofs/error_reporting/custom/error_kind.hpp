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

#ifndef IOX_HOOFS_ERROR_REPORTING_CUSTOM_ERROR_KIND_HPP
#define IOX_HOOFS_ERROR_REPORTING_CUSTOM_ERROR_KIND_HPP

#include "iceoryx_hoofs/error_reporting/error_kind.hpp"

// ***
// * Extend error kinds
// ***

namespace iox
{
namespace er
{

// The non-fatal error kinds can all be defined here.

struct RuntimeErrorKind
{
    static constexpr char const* name = "Runtime Error";
};

constexpr RuntimeErrorKind RUNTIME_ERROR{};

} // namespace er
} // namespace iox

#endif
