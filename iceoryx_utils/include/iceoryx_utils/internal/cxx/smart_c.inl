// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

namespace iox
{
namespace cxx
{
template <typename Function, typename ReturnType, typename... FunctionArguments>
inline SmartC<Function, ReturnType, FunctionArguments...>
makeSmartCImpl(const char* file,
               const int line,
               const char* func,
               const Function& f_function,
               const ReturnMode& f_mode,
               const std::initializer_list<ReturnType>& f_returnValues,
               const std::initializer_list<int>& f_ignoredValues,
               FunctionArguments... f_args) noexcept
{
    auto retval = SmartC<Function, ReturnType, FunctionArguments...>(file,
                                                                     line,
                                                                     func,
                                                                     f_function,
                                                                     f_mode,
                                                                     f_returnValues,
                                                                     f_ignoredValues,
                                                                     std::forward<FunctionArguments>(f_args)...);

    bool doIgnoreEINTR = false;
    if (retval.hasErrors())
    {
        for (auto& value : f_ignoredValues)
        {
            if (value == EINTR)
            {
                doIgnoreEINTR = true;
                break;
            }
        }
    }

    if (!doIgnoreEINTR)
    {
        for (int remainingRetrys = 5; retval.hasErrors() && retval.getErrNum() == EINTR && remainingRetrys > 0;
             --remainingRetrys)
        {
            retval = SmartC<Function, ReturnType, FunctionArguments...>(file,
                                                                        line,
                                                                        func,
                                                                        f_function,
                                                                        f_mode,
                                                                        f_returnValues,
                                                                        f_ignoredValues,
                                                                        std::forward<FunctionArguments>(f_args)...);
        }
    }

    if (retval.hasErrors() && retval.getErrNum())
    {
        if (!(doIgnoreEINTR && retval.getErrNum() == EINTR))
        {
            std::cerr << file << ":" << line << " { " << func << " }  :::  [ " << retval.getErrNum() << " ]  "
                      << retval.getErrorString() << std::endl;
        }
    }

    return retval;
}

template <typename Function, typename ReturnType, typename... FunctionArguments>
int SmartC<Function, ReturnType, FunctionArguments...>::resetErrnoAndInitErrnum() noexcept
{
    errno = 0;
    return 0;
}

template <typename Function, typename ReturnType, typename... FunctionArguments>
SmartC<Function, ReturnType, FunctionArguments...>::SmartC(const char* file,
                                                           const int line,
                                                           const char* func,
                                                           const Function& f_function,
                                                           const ReturnMode& f_mode,
                                                           const std::initializer_list<ReturnType>& f_returnValues,
                                                           const std::initializer_list<int>& f_ignoredValues,
                                                           FunctionArguments... f_args) noexcept
    : m_errnum(resetErrnoAndInitErrnum())
    , m_returnValue(f_function(f_args...))
    , m_errorSource{file, line, func}
{
    switch (f_mode)
    {
    case ReturnMode::PRE_DEFINED_ERROR_CODE:
    {
        m_hasErrors = false;
        for (const auto value : f_returnValues)
        {
            if (value == m_returnValue)
            {
                m_errnum = errno;
                strncpy(m_errorString, strerror(errno), m_errorStringSize);
                for (const auto ignored : f_ignoredValues)
                {
                    if (m_errnum == ignored)
                    {
                        return;
                    }
                }
                m_hasErrors = true;

                if (m_errnum != EINTR)
                {
                    std::cerr << m_errorSource.file << ":" << m_errorSource.line << " { " << m_errorSource.func
                              << " }  :::  [ " << m_errnum << " ]  " << m_errorString << std::endl;
                }

                break;
            }
        }
        break;
    }

    case ReturnMode::PRE_DEFINED_SUCCESS_CODE:
    {
        m_hasErrors = true;
        for (const auto value : f_returnValues)
        {
            if (value == m_returnValue)
            {
                m_hasErrors = false;
                return;
            }
        }
        m_errnum = errno;
        strncpy(m_errorString, strerror(errno), m_errorStringSize);

        for (const auto ignored : f_ignoredValues)
        {
            if (m_errnum == ignored)
            {
                m_hasErrors = false;
                return;
            }
        }

        if (m_errnum != EINTR)
        {
            std::cerr << m_errorSource.file << ":" << m_errorSource.line << " { " << m_errorSource.func << " }  :::  [ "
                      << m_returnValue << " ]  " << m_errorString << std::endl;
        }
        break;
    }
    }
}


template <typename Function, typename ReturnType, typename... FunctionArguments>
inline ReturnType SmartC<Function, ReturnType, FunctionArguments...>::getReturnValue() const noexcept
{
    return m_returnValue;
}

template <typename Function, typename ReturnType, typename... FunctionArguments>
inline SmartC<Function, ReturnType, FunctionArguments...>::operator ReturnType() const noexcept
{
    return getReturnValue();
}

template <typename Function, typename ReturnType, typename... FunctionArguments>
inline bool SmartC<Function, ReturnType, FunctionArguments...>::hasErrors() const noexcept
{
    return m_hasErrors;
}

template <typename Function, typename ReturnType, typename... FunctionArguments>
inline const char* SmartC<Function, ReturnType, FunctionArguments...>::getErrorString() const noexcept
{
    return m_errorString;
}

template <typename Function, typename ReturnType, typename... FunctionArguments>
inline int SmartC<Function, ReturnType, FunctionArguments...>::getErrNum() const noexcept
{
    return m_errnum;
}
} // namespace cxx
} // namespace iox
