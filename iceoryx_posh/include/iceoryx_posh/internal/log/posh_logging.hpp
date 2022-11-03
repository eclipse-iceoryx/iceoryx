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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_LOG_POSH_LOGGING_HPP
#define IOX_POSH_LOG_POSH_LOGGING_HPP

// @todo iox-#1755 for transition only; delete this file

#include "iceoryx_hoofs/log/logging.hpp"

#define LogFatal() IOX_LOG(FATAL)
#define LogError() IOX_LOG(ERROR)
#define LogWarn() IOX_LOG(WARN)
#define LogInfo() IOX_LOG(INFO)
#define LogDebug() IOX_LOG(DEBUG)
#define LogTrace() IOX_LOG(TRACE)
#define LogVerbose() IOX_LOG(TRACE)

#endif // IOX_POSH_LOG_POSH_LOGGING_HPP
