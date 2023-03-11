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

// Awfully complicated macros to get around some limitation with IOX_PANIC(...)
// Can this be simplified? We can drop it altogether at the loss of some API flexibility.
// But it is educational...

// For testing and general purpose...
#define EXPAND_STR_(x) STR(x)
#define STR_(...) #__VA_ARGS__
#define EXPAND_(...) __VA_ARGS__

#define ARG3_(_1, _2, x, ...) x

// Expands to 0 if it has no comma, 1 otherwise for up to 3 args
// This distinguishes between two cases: i) 0 or 1 arguments ii) >=2 arguments
// But the tricky part to distinguish between 0 and 1 arguments is yet to come ...
#define HAS_COMMA_(...) ARG3_(__VA_ARGS__, 1, 0)

// This is used in trickery that relies on left to right macro expansion (and other rules).
// It is used in a way that expands to a comma if and only if X is empty, like so
// COMMA_IF_PARENS X (). Relies on macro expansion rules.
#define COMMA_IF_PARENS_(...) ,

// Corresponds to the 4 checks in IS_EMPTY (first is the main case, rest are corner cases).
// The check results are concatenated and only in the 0001 case we expand to a (otherwise nothing).
// Finally is is expanded to 1 as it has a comma in the 0001 case or 0 otherwise.
#define EMPTY_CASE_0001 ,
#define CONCAT_(_1, _2, _3, _4, _5) _1##_2##_3##_4##_5
#define IS_EMPTY_(_1, _2, _3, _4) HAS_COMMA_(CONCAT_(EMPTY_CASE_, _1, _2, _3, _4))

// The main check macro, expands to 1 if argument list is empty and 0 otherwise.
#define IS_EMPTY(...)                                                                                                  \
    IS_EMPTY_(HAS_COMMA_(__VA_ARGS__),                                                                                 \
              HAS_COMMA_(COMMA_IF_PARENS_ __VA_ARGS__),                                                                \
              HAS_COMMA_(__VA_ARGS__(/*empty parens*/)),                                                               \
              HAS_COMMA_(COMMA_IF_PARENS_ __VA_ARGS__(/*empty parens*/)))

// Now we can define a new panic macro that works correctly with 0 or 1 argument.
// (1 or more arguments is trivial in comparison ...)
// Arguably a defect in the language, easily solved with a GCC extension using ##__VA_ARGS__ ...

#define IOX_PANIC_(empty_check_result, ...) EXPAND_PANIC_(empty_check_result, __VA_ARGS__)
#define EXPAND_PANIC_(empty_check_result, ...) PANIC_CASE_##empty_check_result(__VA_ARGS__)

// empty case (check yields 0 which is concatenated)
#define PANIC_CASE_0(...) iox::err::panic(CURRENT_SOURCE_LOCATION, __VA_ARGS__)
// empty case (check yields 1 which is concatenated)
#define PANIC_CASE_1(...) iox::err::panic(CURRENT_SOURCE_LOCATION)

// Easy peasy ...
#define IOX_PANIC_ISO_CPP_COMPLIANT_(...) IOX_PANIC_(IS_EMPTY(__VA_ARGS__), __VA_ARGS__)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif
