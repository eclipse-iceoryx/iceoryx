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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_DDS_LOG_LOGGING_HPP
#define IOX_DDS_LOG_LOGGING_HPP

#include "iceoryx_utils/log/logging_free_function_building_block.hpp"

namespace iox
{
namespace dds
{
struct DDSLoggingComponent
{
    static constexpr char Ctx[] = "DDS";
    static constexpr char Description[] = "Log context of the DDS module.";
};

static constexpr auto LogFatal = iox::log::ffbb::LogFatal<DDSLoggingComponent>;
static constexpr auto LogError = iox::log::ffbb::LogError<DDSLoggingComponent>;
static constexpr auto LogWarn = iox::log::ffbb::LogWarn<DDSLoggingComponent>;
static constexpr auto LogInfo = iox::log::ffbb::LogInfo<DDSLoggingComponent>;
static constexpr auto LogDebug = iox::log::ffbb::LogDebug<DDSLoggingComponent>;
static constexpr auto LogVerbose = iox::log::ffbb::LogVerbose<DDSLoggingComponent>;

} // namespace dds
} // namespace iox

#endif // IOX_DDS_LOG_LOGGING_HPP
