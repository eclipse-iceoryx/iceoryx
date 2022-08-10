
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

#include "iceoryx_hoofs/log/platform_building_blocks/logger.hpp"
#include "iceoryx_hoofs/log/platform_building_blocks/logcommon.hpp"

#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace iox
{
namespace pbb
{
LogLevel logLevelFromEnvOr(const LogLevel logLevel) noexcept
{
    auto specifiedLogLevel = logLevel;

    // AXIVION Next Construct AutosarC++19_03-M18.0.3 : Use of getenv is allowed in MISRA amendment#6312
    // JUSTIFICATION getenv is required for the functionality of this function; see also declaration in header
    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    if (const auto* logLevelString = std::getenv("IOX_LOG_LEVEL"))
    {
        if (equalStrings(logLevelString, "off"))
        {
            specifiedLogLevel = LogLevel::OFF;
        }
        else if (equalStrings(logLevelString, "fatal"))
        {
            specifiedLogLevel = LogLevel::FATAL;
        }
        else if (equalStrings(logLevelString, "error"))
        {
            specifiedLogLevel = LogLevel::ERROR;
        }
        else if (equalStrings(logLevelString, "warn"))
        {
            specifiedLogLevel = LogLevel::WARN;
        }
        else if (equalStrings(logLevelString, "info"))
        {
            specifiedLogLevel = LogLevel::INFO;
        }
        else if (equalStrings(logLevelString, "debug"))
        {
            specifiedLogLevel = LogLevel::DEBUG;
        }
        else if (equalStrings(logLevelString, "trace"))
        {
            specifiedLogLevel = LogLevel::TRACE;
        }
        else
        {
            puts("Invalide value for 'IOX_LOG_LEVEL' environment variable!'");
        }
    }
    return specifiedLogLevel;
}

} // namespace pbb
} // namespace iox
