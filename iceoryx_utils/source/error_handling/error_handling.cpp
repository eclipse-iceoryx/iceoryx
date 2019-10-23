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

#include "iceoryx_utils/error_handling/error_handling.hpp"

#include "ac3log/simplelogger.hpp"

#include <iostream>
#include <sstream>

namespace iox
{
const char* ErrorHandler::errorNames[] = {ICEORYX_ERRORS(CREATE_ICEORYX_ERROR_STRING)};

HandlerFunction ErrorHandler::handler = {ErrorHandler::DefaultHandler};

std::mutex ErrorHandler::handler_mutex;

void ErrorHandler::DefaultHandler(const Error error,
                                  const std::function<void()> errorCallBack,
                                  const ErrorLevel level)
{
    if (errorCallBack)
    {
        errorCallBack();
    }
    else
    {
        std::stringstream ss;
        ss << "ICEORYX error! " << ErrorHandler::ToString(error);

        ReactOnErrorLevel(level, ss.str().c_str());
    }
}

void ErrorHandler::ReactOnErrorLevel(const ErrorLevel level, const char* errorText)
{
    switch (level)
    {
    case ErrorLevel::FATAL:
        LOG_ERR(errorText);
        assert(false);
        std::terminate();
        break;
    case ErrorLevel::SEVERE:
        LOG_WARN(errorText);
        assert(false);
        break;
    case ErrorLevel::MODERATE:
        LOG_WARN(errorText);
        break;
    }
}

cxx::GenericRAII ErrorHandler::SetTemporaryErrorHandler(const HandlerFunction& newHandler)
{
    return cxx::GenericRAII(
        [&newHandler] {
            std::lock_guard<std::mutex> lock(handler_mutex);
            handler = newHandler;
        },
        [] {
            std::lock_guard<std::mutex> lock(handler_mutex);
            handler = DefaultHandler;
        });
}


const char* ErrorHandler::ToString(const Error error)
{
    return ErrorHandler::errorNames[static_cast<uint32_t>(error)];
}

void errorHandler(const Error error,
                  const std::function<void()> errorCallBack,
                  const ErrorLevel level)
{
    ErrorHandler::handler(error, errorCallBack, level);
}
} // namespace iox
