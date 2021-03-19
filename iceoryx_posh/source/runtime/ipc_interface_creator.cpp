// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

namespace iox
{
namespace runtime
{
IpcInterfaceCreator::IpcInterfaceCreator(const ProcessName_t& name,
                                         const uint64_t maxMessages,
                                         const uint64_t messageSize) noexcept
    : IpcInterfaceBase(name, maxMessages, messageSize)
    , m_fileLock(std::move(posix::FileLock::create(name)
                               .or_else([](auto& error) {
                                   if (error == posix::FileLockError::LOCKED_BY_OTHER_PROCESS)
                                   {
                                       // If the lock couldn't be acquired, some application with the same name is still
                                       // running
                                       LogError()
                                           << "An application with the same runtime name is still running. Using the "
                                              "same runtime name twice is not supported.";
                                       errorHandler(Error::kPOSH__RUNTIME_APP_WITH_SAME_RUNTIME_NAME_STILL_RUNNING,
                                                    nullptr,
                                                    iox::ErrorLevel::FATAL);
                                   }
                                   else
                                   {
                                       LogError() << "Error :"; /// @todo << error;
                                   }
                               })
                               .value()))
{
    // check if the IPC channel is still there (e.g. because of no proper termination
    // of the process)
    cleanupOutdatedIpcChannel(name);

    openIpcChannel(posix::IpcChannelSide::SERVER);
}

void IpcInterfaceCreator::cleanupResource()
{
    m_ipcChannel.destroy();
}
} // namespace runtime
} // namespace iox
