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
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/filesystem.hpp"

namespace iox
{
namespace runtime
{
expected<IpcInterfaceCreator, IpcInterfaceCreatorError> IpcInterfaceCreator::create(const RuntimeName_t& runtimeName,
                                                                                    const DomainId domainId,
                                                                                    const ResourceType resourceType,
                                                                                    const uint64_t maxMessages,
                                                                                    const uint64_t messageSize) noexcept
{
    auto interfaceName = ipcChannelNameToInterfaceName(runtimeName, domainId, resourceType);
    auto fileLock =
        FileLockBuilder().name(interfaceName).permission(iox::perms::owner_read | iox::perms::owner_write).create();

    if (fileLock.has_error())
    {
        switch (fileLock.error())
        {
        case FileLockError::LOCKED_BY_OTHER_PROCESS:
            return err(IpcInterfaceCreatorError::INTERFACE_IN_USE);
        default:
            return err(IpcInterfaceCreatorError::OBTAINING_LOCK_FAILED);
        }
    }

    // remove outdated IPC channel, e.g. because of no proper termination of the process
    cleanupOutdatedIpcChannel(interfaceName);

    return ok(IpcInterfaceCreator{
        std::move(fileLock.value()), runtimeName, domainId, resourceType, maxMessages, messageSize});
}

IpcInterfaceCreator::IpcInterfaceCreator(FileLock&& fileLock,
                                         const RuntimeName_t& runtimeName,
                                         const DomainId domainId,
                                         const ResourceType resourceType,
                                         const uint64_t maxMessages,
                                         const uint64_t messageSize) noexcept
    : IpcInterfaceBase(runtimeName, domainId, resourceType, maxMessages, messageSize)
    , m_fileLock(std::move(fileLock))
{
    openIpcChannel(PosixIpcChannelSide::SERVER);
}
} // namespace runtime
} // namespace iox
