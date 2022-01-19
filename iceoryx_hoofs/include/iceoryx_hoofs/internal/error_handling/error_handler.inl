// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_INL
#define IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_INL

namespace iox
{
template <typename Error>
std::mutex ErrorHandler<Error>::handler_mutex;

template <typename Error>
inline void ErrorHandler<Error>::reactOnErrorLevel(const ErrorLevel level, const char* errorText) noexcept
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

template <typename Error>
inline cxx::GenericRAII ErrorHandler<Error>::setTemporaryErrorHandler(const HandlerFunction<Error>& newHandler) noexcept
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
} // namespace iox

#endif // IOX_HOOFS_ERROR_HANDLING_ERROR_HANDLER_INL
