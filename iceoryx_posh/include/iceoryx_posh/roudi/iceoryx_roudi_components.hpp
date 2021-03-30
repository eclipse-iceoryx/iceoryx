// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_ICEORYX_ROUDI_COMPONENTS_HPP
#define IOX_POSH_ROUDI_ICEORYX_ROUDI_COMPONENTS_HPP

#include "iceoryx_posh/internal/roudi/port_manager.hpp"
#include "iceoryx_posh/roudi/memory/iceoryx_roudi_memory_manager.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/posix_wrapper/file_lock.hpp"


namespace iox
{
namespace roudi
{
constexpr char ROUDI_LOCK_NAME[] = "unique-roudi";
struct IceOryxRouDiComponents
{
  public:
    IceOryxRouDiComponents(const RouDiConfig_t& roudiConfig) noexcept;

    virtual ~IceOryxRouDiComponents() = default;

    posix::FileLock fileLock = std::move(posix::FileLock::create(ROUDI_LOCK_NAME)
                                             .or_else([](auto& error) {
                                                 if (error == posix::FileLockError::LOCKED_BY_OTHER_PROCESS)
                                                 {
                                                     LogFatal() << "Could not acquire lock, is RouDi still running?";
                                                     exit(EXIT_FAILURE);
                                                 }
                                                 else
                                                 {
                                                     LogFatal() << "Error occured while acquiring file lock named "
                                                                << ROUDI_LOCK_NAME;
                                                     exit(EXIT_FAILURE);
                                                 }
                                             })
                                             .value());

    /// @brief Handles MemoryProvider and MemoryBlocks
    IceOryxRouDiMemoryManager rouDiMemoryManager;

    /// @brief Handles the ports in shared memory
    PortManager portManager;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_ICEORYX_ROUDI_COMPONENTS_HPP
