// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/error_handling/error_handler.hpp"

namespace iox
{
// NOLINTNEXTLINE(cert-err58-cpp) ErrorHander only used in tests
iox::HandlerFunction ErrorHandler::handler = {ErrorHandler::defaultHandler};

void ErrorHandler::defaultHandler(const uint32_t, const char* errorName, const ErrorLevel level) noexcept
{
    reactOnErrorLevel(level, errorName);
}

void ErrorHandler::reactOnErrorLevel(const ErrorLevel level, const char* errorName) noexcept
{
    constexpr const char ERROR_TEXT[] = "ICEORYX error! ";

    switch (level)
    {
    case ErrorLevel::FATAL:
        LogError() << ERROR_TEXT << errorName;
        assert(false);
        std::terminate();
        break;
    case ErrorLevel::SEVERE:
        LogWarn() << ERROR_TEXT << errorName;
        assert(false);
        break;
    case ErrorLevel::MODERATE:
        LogWarn() << ERROR_TEXT << errorName;
        break;
    }
}

} // namespace iox
