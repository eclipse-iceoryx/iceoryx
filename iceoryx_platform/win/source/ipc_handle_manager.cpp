// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_platform/ipc_handle_manager.hpp"

IpcHandleManager& IpcHandleManager::getInstance() noexcept
{
    static IpcHandleManager instance;
    return instance;
}

IpcHandleManager::~IpcHandleManager() noexcept
{
    for (auto& handle : ipcHandles)
    {
        if (handle.second.ownerShip == OwnerShip::LOAN)
        {
            CloseHandle(handle.second.handle);
        }
    }
}

bool IpcHandleManager::getHandle(const UniqueSystemId& id, HANDLE& handle) const noexcept
{
    std::lock_guard<std::mutex> lock(mtx);

    auto iter = ipcHandles.find(id);
    if (iter != ipcHandles.end())
    {
        handle = iter->second.handle;
        return true;
    }
    return false;
}

void IpcHandleManager::addHandle(const UniqueSystemId& id, const OwnerShip ownerShip, HANDLE handle) noexcept
{
    std::lock_guard<std::mutex> lock(mtx);

    ipcHandles[id] = IpcHandle_t{ownerShip, handle};
}

void IpcHandleManager::removeHandle(const UniqueSystemId& id) noexcept
{
    std::lock_guard<std::mutex> lock(mtx);

    auto iter = ipcHandles.find(id);
    if (iter != ipcHandles.end())
    {
        ipcHandles.erase(iter);
    }
}
