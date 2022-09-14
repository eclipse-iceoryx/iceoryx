// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_PLATFORM_LOG_LOGGER_HPP
#define IOX_HOOFS_PLATFORM_LOG_LOGGER_HPP

#include "iceoryx_hoofs/log/platform_building_blocks/console_logger.hpp"
#include "iceoryx_hoofs/log/platform_building_blocks/logger.hpp"

namespace iox
{
namespace platform
{
using LogLevel = pbb::LogLevel;
using pbb::asStringLiteral;
using pbb::logLevelFromEnvOr;

using Logger = pbb::Logger<pbb::ConsoleLogger>;
using TestingLoggerBase = pbb::Logger<pbb::ConsoleLogger>;

/// @todo iox-#1345 make this a compile time option since if will reduce performance but some logger might want
/// to do the filtering by themselves
static constexpr bool IGNORE_ACTIVE_LOG_LEVEL{false};

/// @todo iox-#1345 compile time option for minimal compiled log level, i.e. all lower log level should be
/// optimized out; this is different than IGNORE_ACTIVE_LOG_LEVEL since the active log level could still be set to off
static constexpr LogLevel MINIMAL_LOG_LEVEL{LogLevel::TRACE};

} // namespace platform
} // namespace iox

#endif // IOX_HOOFS_PLATFORM_LOG_LOGGER_HPP
