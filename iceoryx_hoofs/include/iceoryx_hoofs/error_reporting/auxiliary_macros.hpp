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

// It is difficult to properly distinguish between 0 and 1 argument,
// MACRO(...) invoked with MACRO() has __VA_ARGS__ contain a single empty token
//
// However, it is not needed here as we can assume we have at least one argument in any correct use.
// Note that in contrast, for MACRO(x, ...) variadic args cannot be empty according to ISO C++.
// All macros assume that there is at least one argument.

// These are internal macros, as indicated by trailing _
// With these helper macros, variadic macros do not rely on GCC extensions anymore and are ISO compliant.

// NOLINTBEGIN(cppcoreguidelines-macro-usage) source location requires macros

#ifndef IOX_HOOFS_ERROR_REPORTING_AUXILIARY_MACROS_HPP
#define IOX_HOOFS_ERROR_REPORTING_AUXILIARY_MACROS_HPP

// Expands to the first argument and discards all others.
#define FIRST_ARG_(...) FIRST_(__VA_ARGS__, discarded)
#define FIRST_(x, ...) x

// Expands to second argument if it exists and discard all others.
// If it does not exist (i.e. there is just one argument) expands to "".
#define SECOND_ARG_OR_EMPTY_(...) SECOND_(__VA_ARGS__, "")
#define SECOND_(_1, x, ...) x

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif
