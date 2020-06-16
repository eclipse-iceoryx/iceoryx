// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/waitset/guard_condition.hpp"
#include "iceoryx_posh/internal/popo/waitset/wait_set.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "test.hpp"

#include <memory>

using namespace ::testing;
using ::testing::Return;
using namespace iox::popo;
using namespace iox::cxx;
using namespace iox::mepoo;

class WaitSet_test : public Test
{
  public:
    static constexpr size_t MEGABYTE = 1 << 20;
    static constexpr size_t MEMORY_SIZE = 1 * MEGABYTE;
    const uint64_t HISTORY_SIZE = 16;
    static constexpr uint32_t MAX_NUMBER_QUEUES = 128;
    char memory[MEMORY_SIZE];
    iox::posix::Allocator allocator{memory, MEMORY_SIZE};
    MemPool mempool{128, 20, &allocator, &allocator};
    MemPool chunkMgmtPool{128, 20, &allocator, &allocator};

    WaitSet m_waitset;
    GuardCondition m_guardCondition;

    void SetUp(){};
    void TearDown(){};
};

// TEST_F(WaitSet_test, NoAttachResultsInError)
// {
// }
