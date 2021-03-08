// Copyright (c) 2021 by  Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_MOCKS_ROUDI_MEMORY_INTERFACE_MOCK_HPP
#define IOX_POSH_MOCKS_ROUDI_MEMORY_INTERFACE_MOCK_HPP

#include "iceoryx_posh/roudi/memory/roudi_memory_interface.hpp"

#include "test.hpp"

using namespace iox::roudi;
using namespace iox::cxx;
using namespace iox::mepoo;

class RouDiMemoryInterfaceMock : public iox::roudi::RouDiMemoryInterface
{
  public:
    MOCK_METHOD0(createAndAnnounceMemoryImpl, expected<RouDiMemoryManagerError>());
    virtual expected<RouDiMemoryManagerError> createAndAnnounceMemory() noexcept override
    {
        return createAndAnnounceMemoryImpl();
    }

    MOCK_METHOD0(destroyMemoryImpl, expected<RouDiMemoryManagerError>());
    virtual expected<RouDiMemoryManagerError> destroyMemory() noexcept override
    {
        return destroyMemoryImpl();
    }

    MOCK_CONST_METHOD0(mgmtMemoryProviderImpl, const PosixShmMemoryProvider*());
    virtual const PosixShmMemoryProvider* mgmtMemoryProvider() const noexcept override
    {
        return mgmtMemoryProviderImpl();
    }

    MOCK_METHOD0(portPoolImpl, optional<PortPool*>());
    virtual optional<PortPool*> portPool() noexcept override
    {
        return portPoolImpl();
    }

    MOCK_CONST_METHOD0(introspectionMemoryManagerImpl, optional<MemoryManager*>());
    virtual optional<MemoryManager*> introspectionMemoryManager() const noexcept override
    {
        return introspectionMemoryManagerImpl();
    }

    MOCK_CONST_METHOD0(segmentManagerImpl, optional<SegmentManager<>*>());
    virtual optional<SegmentManager<>*> segmentManager() const noexcept override
    {
        return segmentManagerImpl();
    }
};


#endif
