// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/error_handling/error_handling.hpp"

#include "iceoryx_hoofs/log/logger.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/log/logmanager.hpp"

#include <iostream>
#include <sstream>

namespace iox
{
const char* ErrorHandler::ERROR_NAMES[] = {ICEORYX_ERRORS(CREATE_ICEORYX_ERROR_STRING)};

// NOLINTNEXTLINE(cert-err58-cpp) ErrorHander only used in tests
HandlerFunction ErrorHandler::handler = {ErrorHandler::defaultHandler};

std::mutex ErrorHandler::handler_mutex;

std::ostream& operator<<(std::ostream& stream, Error value) noexcept
{
    stream << ErrorHandler::toString(value);
    return stream;
}

void ErrorHandler::defaultHandler(const Error error,
                                  const std::function<void()>& errorCallBack,
                                  const ErrorLevel level) noexcept
{
    if (errorCallBack)
    {
        errorCallBack();
    }
    else
    {
        std::stringstream ss;
        ss << "ICEORYX error! " << error;

        reactOnErrorLevel(level, ss.str().c_str());
    }
}

void ErrorHandler::reactOnErrorLevel(const ErrorLevel level, const char* errorText) noexcept
{
    static auto& logger = createLogger("", "", log::LogManager::GetLogManager().DefaultLogLevel());
    switch (level)
    {
    case ErrorLevel::FATAL:
        logger.LogError() << errorText;
        assert(false);
        std::terminate();
        break;
    case ErrorLevel::SEVERE:
        logger.LogWarn() << errorText;
        assert(false);
        break;
    case ErrorLevel::MODERATE:
        logger.LogWarn() << errorText;
        break;
    }
}

cxx::GenericRAII ErrorHandler::setTemporaryErrorHandler(const HandlerFunction& newHandler) noexcept
{
    return cxx::GenericRAII(
        [&newHandler] {
            std::lock_guard<std::mutex> lock(handler_mutex);
            handler = newHandler;
        },
        [] {
            std::lock_guard<std::mutex> lock(handler_mutex);
            handler = defaultHandler;
        });
}


const char* ErrorHandler::toString(const Error error) noexcept
{
    return ErrorHandler::ERROR_NAMES[static_cast<uint32_t>(error)];
}

void errorHandler(const Error error, const std::function<void()>& errorCallBack, const ErrorLevel level) noexcept
{
    ErrorHandler::handler(error, errorCallBack, level);
}
} // namespace iox
