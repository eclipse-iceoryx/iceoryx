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

#ifndef IOX_HOOFS_ERROR_REPORTING_ERROR_KIND_HPP
#define IOX_HOOFS_ERROR_REPORTING_ERROR_KIND_HPP

#include <type_traits>

namespace iox
{
namespace err
{

// mandatory fatal error category that always exists
struct Fatal
{
    static constexpr char const* name = "Fatal Error";
};

struct PreconditionViolation
{
    static constexpr char const* name = "Precondition Violation";
};

// postconditions and other asserts, not for preconditions
struct DebugAssertViolation
{
    static constexpr char const* name = "DebugAssert Violation";
};

template <class T>
struct IsFatal : public std::false_type
{
};

// This specialization makes it impossible to specialize them differently elsewhere,
// as this would lead to a compilation error.
// This enforces that these errors are always fatal in the sense that they cause panic and abort.
template <>
struct IsFatal<Fatal> : public std::true_type
{
};

template <>
struct IsFatal<PreconditionViolation> : public std::true_type
{
};

template <>
struct IsFatal<DebugAssertViolation> : public std::true_type
{
};

// The function syntax is more useful if there is already a value (instead of only a type).
// It must be consistent with the type trait, i.e. yield the same boolean value.
template <class Kind>
bool constexpr isFatal(Kind)
{
    return IsFatal<Kind>::value;
}

template <>
bool constexpr isFatal<Fatal>(Fatal)
{
    return IsFatal<Fatal>::value;
}

template <>
bool constexpr isFatal<PreconditionViolation>(PreconditionViolation)
{
    return IsFatal<PreconditionViolation>::value;
}

template <>
bool constexpr isFatal<DebugAssertViolation>(DebugAssertViolation)
{
    return IsFatal<DebugAssertViolation>::value;
}

// indicates serious condition, unable to continue
constexpr Fatal FATAL;

// indicates a bug (contract breach by caller)
constexpr PreconditionViolation PRECONDITION_VIOLATION;

// indicates a bug (contract breach by callee)
constexpr DebugAssertViolation DEBUG_ASSERT_VIOLATION;

} // namespace err
} // namespace iox

#endif
