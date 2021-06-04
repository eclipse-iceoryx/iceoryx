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
#ifndef IOX_HOOFS_WIN_PLATFORM_IPC_HANDLE_MANAGER_HPP
#define IOX_HOOFS_WIN_PLATFORM_IPC_HANDLE_MANAGER_HPP

#include "iceoryx_hoofs/platform/unique_system_id.hpp"
#include "iceoryx_hoofs/platform/windows.hpp"
#include <map>


enum class OwnerShip
{
    OWN,
    LOAN,
};

struct IpcHandle_t
{
    OwnerShip ownerShip = OwnerShip::LOAN;
    HANDLE handle = nullptr;
};

class IpcHandleManager
{
  public:
    ~IpcHandleManager() noexcept;

    bool getHandle(const UniqueSystemId& id, HANDLE& handle) noexcept;
    void addHandle(const UniqueSystemId& id, const OwnerShip ownerShip, HANDLE handle) noexcept;
    void removeHandle(const UniqueSystemId& id) noexcept;

  private:
    std::map<UniqueSystemId, IpcHandle_t> ipcHandles;
};

#endif
