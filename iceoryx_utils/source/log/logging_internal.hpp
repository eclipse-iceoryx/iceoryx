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

#pragma once

#include "iceoryx_utils/log/logging_free_function_building_block.hpp"

namespace iox
{
namespace log
{
struct LoggingComponent
{
    static constexpr char Ctx[] = "IceoryxUtils";
    static constexpr char Description[] = "Log context of the IceoryxUtils component!";
};

static constexpr auto LogFatal = iox::log::ffbb::LogFatal<LoggingComponent>;
static constexpr auto LogError = iox::log::ffbb::LogError<LoggingComponent>;
static constexpr auto LogWarn = iox::log::ffbb::LogWarn<LoggingComponent>;
static constexpr auto LogInfo = iox::log::ffbb::LogInfo<LoggingComponent>;
static constexpr auto LogDebug = iox::log::ffbb::LogDebug<LoggingComponent>;
static constexpr auto LogVerbose = iox::log::ffbb::LogVerbose<LoggingComponent>;

} // namespace log
} // namespace iox

