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

#include "iceoryx_utils/fixed_string/string100.hpp"

namespace iox
{
namespace runtime
{
/// @brief struct which contains all the members of an object of type Runnable
struct RunnableData
{
    /// @brief constructor
    /// @param[in] name name of the runnable
    /// @param[in] runnableDeviceIdentifier identifier of the device on which the runnable will run
    RunnableData(const iox::cxx::CString100& process, const iox::cxx::CString100& runnable, const uint64_t runnableDeviceIdentifier) noexcept;

    RunnableData(const RunnableData&) = delete;
    RunnableData(RunnableData&&) = delete;
    RunnableData& operator=(const RunnableData&) = delete;
    RunnableData& operator=(RunnableData&&) = delete;


    iox::cxx::CString100 m_process;
    iox::cxx::CString100 m_runnable;
    uint64_t m_runnableDeviceIdentifier;
};
} // namespace runtime
} // namespace iox
