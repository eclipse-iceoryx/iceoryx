// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"

#include "iceoryx_posh/roudi/memory/memory_block.hpp"

class MemoryBlockMock final : public iox::roudi::MemoryBlock
{
  public:
    /// @note with gmock 1.8 it is not possible to mock functions with the noexcept specifier, therefore this
    /// interesting mock implementation

    uint64_t size() const noexcept override
    {
        return sizeMock();
    }

    uint64_t alignment() const noexcept override
    {
        return alignmentMock();
    }

    void memoryAvailable(void* memory) noexcept override
    {
        memoryAvailableMock(memory);
    }

    void destroy() noexcept override
    {
        destroyMock();
    }

    MOCK_CONST_METHOD0(sizeMock, uint64_t());
    MOCK_CONST_METHOD0(alignmentMock, uint64_t());
    MOCK_METHOD1(memoryAvailableMock, void(void*));
    MOCK_METHOD0(destroyMock, void());
};
