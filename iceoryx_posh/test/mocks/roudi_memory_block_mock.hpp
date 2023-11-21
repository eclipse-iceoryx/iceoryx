// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_MOCKS_ROUDI_MEMORY_BLOCK_MOCK_HPP
#define IOX_POSH_MOCKS_ROUDI_MEMORY_BLOCK_MOCK_HPP

#include "test.hpp"

#include "iceoryx_posh/roudi/memory/memory_block.hpp"
#include "iox/not_null.hpp"

class MemoryBlockMock final : public iox::roudi::MemoryBlock
{
  public:
    MOCK_METHOD(uint64_t, size, (), (const, noexcept, override));
    MOCK_METHOD(uint64_t, alignment, (), (const, noexcept, override));
    MOCK_METHOD(void, onMemoryAvailable, (iox::not_null<void*>), (noexcept, override));
    MOCK_METHOD(void, destroy, (), (noexcept, override));
};

#endif // IOX_POSH_MOCKS_ROUDI_MEMORY_BLOCK_MOCK_HPP
