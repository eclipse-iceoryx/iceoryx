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
// limitations under the License.#include "test.hpp"

#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/internal/roudi/memory/mempool_collection_memory_block.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"
#include "iceoryx_utils/posix_wrapper/posix_access_rights.hpp"
#include "test.hpp"

#include <thread>
namespace
{
using namespace ::testing;

TEST(ShmCreatorDeathTest, AllocatingTooMuchMemoryLeadsToExitWithSIGBUS)
{
    const iox::roudi::ShmNameString TEST_SHM_NAME{"/test_name"};
    // try a config with high memory requirements, expect failure
    iox::mepoo::MePooConfig badconfig;
    badconfig.addMemPool({1 << 30, 100});
    iox::roudi::MemPoolCollectionMemoryBlock badmempools(badconfig);
    iox::roudi::PosixShmMemoryProvider badShmProvider(
        TEST_SHM_NAME, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine);
    badShmProvider.addMemoryBlock(&badmempools);

    EXPECT_DEATH(badShmProvider.create(),
                 "\033\\[0;1;97;41mFatal error:\033\\[m the available memory is insufficient. Cannot allocate mempools "
                 "in shared memory. Please make sure that enough memory is available. For this, consider also the "
                 "memory which is required for the \\[/iceoryx_mgmt\\] segment. Please refer to "
                 "share\\/doc\\/iceoryx\\/FAQ.md in your release delivery.");

    // try again with a config with low memory requirements; success clears shared memory allocated by the OS in e.g.
    // /dev/shm
    iox::mepoo::MePooConfig goodconfig;
    goodconfig.addMemPool({1024, 1});
    iox::roudi::MemPoolCollectionMemoryBlock goodmempools(goodconfig);
    iox::roudi::PosixShmMemoryProvider goodShmProvider(
        TEST_SHM_NAME, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine);
    goodShmProvider.addMemoryBlock(&goodmempools);
    goodShmProvider.create();
}
} // namespace
