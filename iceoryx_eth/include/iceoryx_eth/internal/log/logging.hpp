// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_eth_LOG_LOGGING_HPP
#define IOX_eth_LOG_LOGGING_HPP

#include "iceoryx_utils/log/logging_free_function_building_block.hpp"

namespace iox
{
namespace eth
{
struct ethLoggingComponent
{
    static constexpr char Ctx[] = "ETH";
    static constexpr char Description[] = "Log context of the ETH module.";
};

static constexpr auto LogFatal = iox::log::ffbb::LogFatal<ethLoggingComponent>;
static constexpr auto LogError = iox::log::ffbb::LogError<ethLoggingComponent>;
static constexpr auto LogWarn = iox::log::ffbb::LogWarn<ethLoggingComponent>;
static constexpr auto LogInfo = iox::log::ffbb::LogInfo<ethLoggingComponent>;
static constexpr auto LogDebug = iox::log::ffbb::LogDebug<ethLoggingComponent>;
static constexpr auto LogVerbose = iox::log::ffbb::LogVerbose<ethLoggingComponent>;

} // namespace eth
} // namespace iox


#endif // IOX_eth_LOG_LOGGING_HPP
