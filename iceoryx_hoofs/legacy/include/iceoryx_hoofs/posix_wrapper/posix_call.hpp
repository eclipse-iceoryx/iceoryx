// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_POSIX_WRAPPER_POSIX_CALL_HPP
#define IOX_HOOFS_POSIX_WRAPPER_POSIX_CALL_HPP

#include "iox/detail/deprecation_marker.hpp"
#include "iox/posix_call.hpp"

IOX_DEPRECATED_HEADER_SINCE(3, "Please include 'iox/posix_call.hpp' instead.")

// clang-format off

namespace iox
{
namespace IOX_DEPRECATED_SINCE(3, "Please use the 'iox' namespace directly and the corresponding header.") posix
{
/// @deprecated use corresponding constant from 'iox/posix_call.hpp'
IOX_DEPRECATED_SINCE(3, "Please use 'POSIX_CALL_ERROR_STRING_SIZE' from 'iox/posix_call.hpp' instead.")
static constexpr uint32_t POSIX_CALL_ERROR_STRING_SIZE = iox::POSIX_CALL_ERROR_STRING_SIZE;
/// @deprecated use corresponding constant from 'iox/posix_call.hpp'
IOX_DEPRECATED_SINCE(3, "Please use 'POSIX_CALL_EINTR_REPETITIONS' from 'iox/posix_call.hpp' instead.")
static constexpr uint64_t POSIX_CALL_EINTR_REPETITIONS = iox::POSIX_CALL_EINTR_REPETITIONS;
/// @deprecated use corresponding constant from 'iox/posix_call.hpp'
IOX_DEPRECATED_SINCE(3, "Please use 'POSIX_CALL_INVALID_ERRNO' from 'iox/posix_call.hpp' instead.")
static constexpr int32_t POSIX_CALL_INVALID_ERRNO = iox::POSIX_CALL_INVALID_ERRNO;

namespace internal
{
// NOLINTJUSTIFICATION just a forwarding function to ensure backwards compatibility
// NOLINTBEGIN(readability-function-size)
template <typename ReturnType, typename... FunctionArguments>
IOX_DEPRECATED_SINCE(3, "Please use 'IOX_POSIX_CALL' from 'iox/posix_call.hpp' instead.")
PosixCallBuilder<ReturnType, FunctionArguments...> createPosixCallBuilder(ReturnType (*posixCall)(FunctionArguments...),
                                                                          const char* posixFunctionName,
                                                                          const char* file,
                                                                          const int32_t line,
                                                                          const char* callingFunction) noexcept
{
    return iox::detail::createPosixCallBuilder(posixCall, posixFunctionName, file, line, callingFunction);
}
// NOLINTEND(readability-function-size)
} // namespace internal

/// @deprecated use 'IOX_POSIX_CALL' from 'iox/posix_call.hpp' instead of 'iox::posix::posixCall'
// NOLINTJUSTIFICATION a template or constexpr function does not have access to source code origin file, line, function
//                     name
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define posixCall(f)                                                                                                   \
    internal::createPosixCallBuilder(                                                                                  \
        &(f),                                                                                                          \
        (#f),                                                                                                          \
        __FILE__,                                                                                                      \
        __LINE__,                                                                                                      \
        __PRETTY_FUNCTION__) // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
// needed for source code location, safely wrapped in macro

} // namespace posix
} // namespace iox

// clang-format on

#endif // IOX_HOOFS_POSIX_WRAPPER_POSIX_CALL_HPP
