// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_UTILS_POSIX_WRAPPER_POSIX_CALL_HPP
#define IOX_UTILS_POSIX_WRAPPER_POSIX_CALL_HPP

#include "iceoryx_utils/cxx/attributes.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/string.hpp"

#include <cstdint>
#include <cstring>

namespace iox
{
namespace posix
{
template <typename ReturnType, typename... FunctionArguments>
class PosixCallBuilder;

template <typename T>
struct PosixCallResult
{
    static constexpr uint32_t ERROR_STRING_SIZE = 128u;
    static const PosixCallResult INVALID_STATE;

    PosixCallResult() noexcept = default;

    cxx::string<ERROR_STRING_SIZE> getHumanReadableErrnum() const noexcept;

    T value;
    int32_t errnum = -1;
};

template <typename T>
PosixCallResult<T> const PosixCallResult<T>::INVALID_STATE{{}, -1};


namespace internal
{
template <typename ReturnType, typename... FunctionArguments>
PosixCallBuilder<ReturnType, FunctionArguments...> createPosixCallBuilder(ReturnType (*posixCall)(FunctionArguments...),
                                                                          const char* posixFunctionName,
                                                                          const char* file,
                                                                          const int32_t line,
                                                                          const char* callingFunction) noexcept;

template <typename ReturnType>
struct PosixCallDetails
{
    const char* posixFunctionName = nullptr;
    const char* file = nullptr;
    int32_t line = 0;
    const char* callingFunction = nullptr;
    bool hasSuccess = true;

    PosixCallResult<ReturnType> result;
};
} // namespace internal

template <typename ReturnType>
class IOX_NO_DISCARD PosixCallEvaluator
{
  public:
    explicit PosixCallEvaluator(internal::PosixCallDetails<ReturnType>& details) noexcept;
    template <typename... IgnoredErrnos>
    cxx::expected<PosixCallResult<ReturnType>, PosixCallResult<ReturnType>>
    evaluateWithIgnoredErrnos(const IgnoredErrnos... ignoredErrnos) const&& noexcept;
    cxx::expected<PosixCallResult<ReturnType>, PosixCallResult<ReturnType>> evaluate() const&& noexcept;

  private:
    internal::PosixCallDetails<ReturnType>& m_details;
};

template <typename ReturnType>
class IOX_NO_DISCARD PosixCallVerificator
{
  public:
    explicit PosixCallVerificator(internal::PosixCallDetails<ReturnType>& details) noexcept;
    PosixCallEvaluator<ReturnType> successReturnValue(const ReturnType value) && noexcept;
    PosixCallEvaluator<ReturnType> failureReturnValue(const ReturnType value) && noexcept;

  private:
    internal::PosixCallDetails<ReturnType>& m_details;
};

template <typename ReturnType, typename... FunctionArguments>
class IOX_NO_DISCARD PosixCallBuilder
{
  public:
    using FunctionType_t = ReturnType (*)(FunctionArguments...);

    PosixCallVerificator<ReturnType> call(FunctionArguments... arguments) && noexcept;

  private:
    template <typename ReturnType_, typename... FunctionArguments_>
    friend PosixCallBuilder<ReturnType_, FunctionArguments_...>
    internal::createPosixCallBuilder(ReturnType_ (*posixCall)(FunctionArguments_...),
                                     const char* posixFunctionName,
                                     const char* file,
                                     const int32_t line,
                                     const char* callingFunction) noexcept;

    PosixCallBuilder(FunctionType_t posixCall,
                     const char* posixFunctionName,
                     const char* file,
                     const int32_t line,
                     const char* callingFunction) noexcept;

  private:
    static constexpr uint64_t EINTR_REPETITIONS = 5U;

    FunctionType_t m_posixCall;
    internal::PosixCallDetails<ReturnType> m_details;
};

#define posixCall(f) internal::createPosixCallBuilder(f, #f, __FILE__, __LINE__, __PRETTY_FUNCTION__)

} // namespace posix
} // namespace iox

#include "iceoryx_utils/internal/posix_wrapper/posix_call.inl"

#endif
