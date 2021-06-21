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
#ifndef IOX_HOOFS_POSIX_WRAPPER_POSIX_CALL_INL
#define IOX_HOOFS_POSIX_WRAPPER_POSIX_CALL_INL

namespace iox
{
namespace posix
{
namespace internal
{
template <typename ReturnType, typename... FunctionArguments>
inline PosixCallBuilder<ReturnType, FunctionArguments...>
createPosixCallBuilder(ReturnType (*posixCall)(FunctionArguments...),
                       const char* posixFunctionName,
                       const char* file,
                       const int32_t line,
                       const char* callingFunction) noexcept
{
    return PosixCallBuilder<ReturnType, FunctionArguments...>(
        posixCall, posixFunctionName, file, line, callingFunction);
}

template <typename ReturnType>
inline PosixCallDetails<ReturnType>::PosixCallDetails(const char* posixFunctionName,
                                                      const char* file,
                                                      int line,
                                                      const char* callingFunction) noexcept
    : posixFunctionName(posixFunctionName)
    , file(file)
    , callingFunction(callingFunction)
    , line(line)
{
}

} // namespace internal

template <typename T>
inline cxx::string<POSIX_CALL_ERROR_STRING_SIZE> PosixCallResult<T>::getHumanReadableErrnum() const noexcept
{
    return cxx::string<POSIX_CALL_ERROR_STRING_SIZE>(cxx::TruncateToCapacity, std::strerror(errnum));
}

template <typename ReturnType, typename... FunctionArguments>
inline PosixCallBuilder<ReturnType, FunctionArguments...>::PosixCallBuilder(FunctionType_t posixCall,
                                                                            const char* posixFunctionName,
                                                                            const char* file,
                                                                            const int32_t line,
                                                                            const char* callingFunction) noexcept
    : m_posixCall{posixCall}
    , m_details{posixFunctionName, file, line, callingFunction}
{
}

template <typename ReturnType, typename... FunctionArguments>
inline PosixCallVerificator<ReturnType>
PosixCallBuilder<ReturnType, FunctionArguments...>::operator()(FunctionArguments... arguments) && noexcept
{
    for (uint64_t i = 0U; i < POSIX_CALL_EINTR_REPETITIONS; ++i)
    {
        errno = 0;
        m_details.result.value = m_posixCall(arguments...);
        m_details.result.errnum = errno;

        if (m_details.result.errnum != EINTR)
        {
            break;
        }
    }

    return PosixCallVerificator<ReturnType>(m_details);
}

template <typename ReturnType>
inline PosixCallVerificator<ReturnType>::PosixCallVerificator(internal::PosixCallDetails<ReturnType>& details) noexcept
    : m_details{details}
{
}

template <typename ReturnType>
template <typename... SuccessReturnValues>
inline PosixCallEvaluator<ReturnType>
PosixCallVerificator<ReturnType>::successReturnValue(const SuccessReturnValues... successReturnValues) && noexcept
{
    m_details.hasSuccess = algorithm::doesContainValue(m_details.result.value, successReturnValues...);

    return PosixCallEvaluator<ReturnType>(m_details);
}

template <typename ReturnType>
template <typename... FailureReturnValues>
inline PosixCallEvaluator<ReturnType>
PosixCallVerificator<ReturnType>::failureReturnValue(const FailureReturnValues... failureReturnValues) && noexcept
{
    using ValueType = decltype(m_details.result.value);
    m_details.hasSuccess =
        !algorithm::doesContainValue(m_details.result.value, static_cast<ValueType>(failureReturnValues)...);

    return PosixCallEvaluator<ReturnType>(m_details);
}

template <typename ReturnType>
inline PosixCallEvaluator<ReturnType> PosixCallVerificator<ReturnType>::returnValueMatchesErrno() && noexcept
{
    m_details.hasSuccess = m_details.result.value == 0;
    m_details.result.errnum = static_cast<int32_t>(m_details.result.value);

    return PosixCallEvaluator<ReturnType>(m_details);
}

template <typename ReturnType>
inline PosixCallEvaluator<ReturnType>::PosixCallEvaluator(internal::PosixCallDetails<ReturnType>& details) noexcept
    : m_details{details}
{
}

template <typename ReturnType>
template <typename... IgnoredErrnos>
inline PosixCallEvaluator<ReturnType>
PosixCallEvaluator<ReturnType>::ignoreErrnos(const IgnoredErrnos... ignoredErrnos) const&& noexcept
{
    if (!m_details.hasSuccess)
    {
        m_details.hasIgnoredErrno |= algorithm::doesContainValue(m_details.result.errnum, ignoredErrnos...);
    }

    return *this;
}

template <typename ReturnType>
template <typename... SilentErrnos>
inline PosixCallEvaluator<ReturnType>
PosixCallEvaluator<ReturnType>::suppressErrorMessagesForErrnos(const SilentErrnos... silentErrnos) const&& noexcept
{
    if (!m_details.hasSuccess)
    {
        m_details.hasSilentErrno |= algorithm::doesContainValue(m_details.result.errnum, silentErrnos...);
    }

    return *this;
}

template <typename ReturnType>
inline cxx::expected<PosixCallResult<ReturnType>, PosixCallResult<ReturnType>>
PosixCallEvaluator<ReturnType>::evaluate() const&& noexcept
{
    if (m_details.hasSuccess || m_details.hasIgnoredErrno)
    {
        return iox::cxx::success<PosixCallResult<ReturnType>>(m_details.result);
    }
    else if (!m_details.hasSilentErrno)
    {
        auto flags = std::cerr.flags();
        std::cerr << m_details.file << ":" << std::dec << m_details.line << " { " << m_details.callingFunction << " -> "
                  << m_details.posixFunctionName << " }  :::  [ " << std::dec << m_details.result.errnum << " ]  "
                  << m_details.result.getHumanReadableErrnum() << std::endl;
        std::cerr.setf(flags);
    }

    return iox::cxx::error<PosixCallResult<ReturnType>>(m_details.result);
}

} // namespace posix
} // namespace iox

#endif // IOX_HOOFS_POSIX_WRAPPER_POSIX_CALL_INL
