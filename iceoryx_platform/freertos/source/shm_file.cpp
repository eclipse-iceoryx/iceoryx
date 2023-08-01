// Copyright (c) 2023 by NXP. All rights reserved.
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

#include <cstdint>
#include <cstring>

#include "iceoryx_platform/shm_file.hpp"

#include "FreeRTOS.h"

std::mutex ShmFile::openFilesMutex;
std::array<ShmFile, ShmFile::MAX_SHM_SEGMENTS> ShmFile::openFiles{};
int ShmFile::s_fdCounter{0};

ShmFile::ShmFile() noexcept
{
    std::memset(m_name, 0x0, ShmFile::MAX_NAME_LENGTH);
}

ShmFile::ShmFile(const char* name) noexcept
    : m_fd(ShmFile::s_fdCounter++)
    , m_full(true)
{
    std::strncpy(m_name, name, ShmFile::MAX_NAME_LENGTH);
}

ShmFile::~ShmFile()
{
    if (m_ptr != nullptr)
    {
        vPortFree(m_ptr);
    }
}

bool ShmFile::ftruncate(uint32_t size) noexcept
{
    m_ptr = pvPortMalloc(size);
    configASSERT(m_ptr != nullptr);
    m_size = size;
    return true;
}

const char* ShmFile::name() const noexcept
{
    return m_name;
}

void* ShmFile::ptr() noexcept
{
    return m_ptr;
}

uint32_t ShmFile::size() const noexcept
{
    return m_size;
}

int ShmFile::fd() const noexcept
{
    return m_fd;
}

bool ShmFile::empty() const noexcept
{
    return !m_full;
}
