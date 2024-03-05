// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_POSH_RUNTIME_IPC_INTERFACE_CREATOR_HPP
#define IOX_POSH_RUNTIME_IPC_INTERFACE_CREATOR_HPP

#include "iceoryx_posh/internal/runtime/ipc_interface_base.hpp"
#include "iox/expected.hpp"
#include "iox/file_lock.hpp"

namespace iox
{
namespace runtime
{
enum class IpcInterfaceCreatorError
{
    INTERFACE_IN_USE,
    OBTAINING_LOCK_FAILED,
};

/// @brief Class for creating and handling a IPC channel
/// @note This class makes sures the IPC channel is created uniquely
class IpcInterfaceCreator : public IpcInterfaceBase
{
  public:
    /// @brief Constructs a 'IpcInterfaceCreator' and opens a new IPC channel.
    /// @param[in] name Unique identifier of the IPC channel
    /// @param[in] domainId to tie the interface to
    /// @param[in] resourceType to be used for the resource prefix
    /// @param[in] maxMessages maximum number of queued messages
    /// @param[in] message size maximum message size
    /// @return The 'IpcInterfaceCreator' or an error if the file lock for the IPC channel could not be obtained
    /// @note The IPC channel might not be initialized. Therefore, 'isInitialized' should always be called before using
    /// this class.
    static expected<IpcInterfaceCreator, IpcInterfaceCreatorError>
    create(const RuntimeName_t& runtimeName,
           const DomainId domainId,
           const ResourceType resourceType,
           const uint64_t maxMessages = ROUDI_MAX_MESSAGES,
           const uint64_t messageSize = ROUDI_MESSAGE_SIZE) noexcept;

    IpcInterfaceCreator(IpcInterfaceCreator&&) noexcept = default;
    IpcInterfaceCreator& operator=(IpcInterfaceCreator&&) noexcept = default;

    /// @brief The copy constructor and assignment operator is deleted since
    ///         this class manages a resource (IPC channel) which cannot
    ///         be copied.
    IpcInterfaceCreator(const IpcInterfaceCreator&) = delete;
    IpcInterfaceCreator& operator=(const IpcInterfaceCreator&) = delete;

  private:
    IpcInterfaceCreator(FileLock&& fileLock,
                        const RuntimeName_t& runtimeName,
                        const DomainId domainId,
                        const ResourceType resourceType,
                        const uint64_t maxMessages,
                        const uint64_t messageSize) noexcept;

  private:
    friend class IpcRuntimeInterface;
    FileLock m_fileLock;
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_IPC_INTERFACE_CREATOR_HPP
