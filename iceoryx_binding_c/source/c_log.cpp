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

#include "iox/logging.hpp"

extern "C" {
#include "iceoryx_binding_c/log.h"
}

using namespace iox::log;

LogLevel toLogLevel(enum iox_LogLevel level)
{
    switch (level)
    {
    case Iceoryx_LogLevel_Off:
        return LogLevel::Off;
    case Iceoryx_LogLevel_Trace:
        return LogLevel::Trace;
    case Iceoryx_LogLevel_Debug:
        return LogLevel::Debug;
    case Iceoryx_LogLevel_Info:
        return LogLevel::Info;
    case Iceoryx_LogLevel_Warn:
        return LogLevel::Warn;
    case Iceoryx_LogLevel_Error:
        return LogLevel::Error;
    case Iceoryx_LogLevel_Fatal:
        return LogLevel::Fatal;
    default:
        return LogLevel::Trace;
    }
}

void iox_set_loglevel(enum iox_LogLevel level)
{
    iox::log::Logger::setLogLevel(toLogLevel(level));
}
