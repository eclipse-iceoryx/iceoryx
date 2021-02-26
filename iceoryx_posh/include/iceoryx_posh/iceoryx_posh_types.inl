// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ICEORYX_POSH_TYPES_INL
#define IOX_POSH_ICEORYX_POSH_TYPES_INL

namespace iox
{
namespace roudi
{
inline iox::log::LogStream& operator<<(iox::log::LogStream& logstream, const MonitoringMode& mode)
{
    switch (mode)
    {
    case MonitoringMode::OFF:
        logstream << "MonitoringMode::OFF";
        break;
    case MonitoringMode::ON:
        logstream << "MonitoringMode::ON";
        break;
    default:
        logstream << "MonitoringMode::UNDEFINED";
        break;
    }
    return logstream;
}
} // namespace roudi

} // namespace iox

#endif // IOX_POSH_ICEORYX_POSH_TYPES_INL
