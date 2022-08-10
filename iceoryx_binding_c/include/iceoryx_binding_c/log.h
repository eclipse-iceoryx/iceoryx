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

#ifndef IOX_BINDING_C_LOG_H
#define IOX_BINDING_C_LOG_H

/// @brief available log levels of the iceoryx runtime
enum iox_LogLevel
{
    Iceoryx_LogLevel_Off = 0,
    Iceoryx_LogLevel_Trace,
    Iceoryx_LogLevel_Debug,
    Iceoryx_LogLevel_Info,
    Iceoryx_LogLevel_Warn,
    Iceoryx_LogLevel_Error,
    Iceoryx_LogLevel_Fatal
};

/// @brief set the log level of the iceoryx runtime
/// @param[in] level log level to be set
/// @note must be called before the runtime is initialized
void iox_set_loglevel(enum iox_LogLevel level);

#endif
