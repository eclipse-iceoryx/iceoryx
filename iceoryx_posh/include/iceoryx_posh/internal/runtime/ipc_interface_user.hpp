// Copyright (c) 2019, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_RUNTIME_IPC_INTERFACE_USER_HPP
#define IOX_POSH_RUNTIME_IPC_INTERFACE_USER_HPP

#include "iceoryx_posh/internal/runtime/ipc_interface_base.hpp"

namespace iox
{
namespace runtime
{
/// @brief Class for handling a message queue via mq_open and mq_close.
class IpcInterfaceUser : public IpcInterfaceBase
{
  public:
    /// @brief Constructs a IpcInterfaceUser and opens a message queue with
    ///         mq_open. If mq_open fails the method isInitialized returns
    ///         false. Therefore, isInitialized should always be called
    ///         before using this class.
    /// @param[in] name Unique identifier of the message queue which
    ///         is used for mq_open
    /// @param[in] maxMessages maximum number of queued messages
    /// @param[in] message size maximum message size
    IpcInterfaceUser(const ProcessName_t& name,
                     const uint64_t maxMessages = APP_MAX_MESSAGES,
                     const uint64_t messageSize = APP_MESSAGE_SIZE) noexcept;

    /// @brief The copy constructor and assignment operator are deleted since
    ///         this class manages a resource (message queue) which cannot
    ///         be copied. Since move is not needed it is also deleted.
    IpcInterfaceUser(const IpcInterfaceUser&) = delete;
    IpcInterfaceUser& operator=(const IpcInterfaceUser&) = delete;

    /// @brief Not needed therefore deleted
    IpcInterfaceUser(IpcInterfaceUser&&) = delete;
    IpcInterfaceUser& operator=(IpcInterfaceUser&&) = delete;
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_IPC_INTERFACE_USER_HPP