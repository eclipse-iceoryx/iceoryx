// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/ipc_interface_creator.hpp"
#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iox/filesystem.hpp"

namespace iox
{
namespace runtime
{
IpcInterfaceCreator::IpcInterfaceCreator(const RuntimeName_t& runtimeName,
                                         const uint64_t maxMessages,
                                         const uint64_t messageSize) noexcept
    : IpcInterfaceBase(runtimeName, maxMessages, messageSize)
    , m_fileLock(std::move(
          posix::FileLockBuilder()
              .name(runtimeName)
              .permission(iox::perms::owner_read | iox::perms::owner_write)
              .create()
              .or_else([&runtimeName](auto& error) {
                  if (error == posix::FileLockError::LOCKED_BY_OTHER_PROCESS)
                  {
                      LogFatal() << "An application with the name " << runtimeName
                                 << " is still running. Using the "
                                    "same name twice is not supported.";
                      errorHandler(PoshError::IPC_INTERFACE__APP_WITH_SAME_NAME_STILL_RUNNING, iox::ErrorLevel::FATAL);
                  }
                  else
                  {
                      LogFatal() << "Error occurred while acquiring file lock named " << runtimeName;
                      errorHandler(PoshError::IPC_INTERFACE__COULD_NOT_ACQUIRE_FILE_LOCK, iox::ErrorLevel::FATAL);
                  }
              })
              .value()))
{
    // check if the IPC channel is still there (e.g. because of no proper termination
    // of the process)
    cleanupOutdatedIpcChannel(runtimeName);

    openIpcChannel(posix::IpcChannelSide::SERVER);
}
} // namespace runtime
} // namespace iox
