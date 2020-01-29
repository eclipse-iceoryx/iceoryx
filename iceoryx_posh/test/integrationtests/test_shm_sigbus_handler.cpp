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

#include "iceoryx_posh/internal/roudi/shared_memory_manager.hpp"
#include "iceoryx_posh/internal/runtime/shared_memory_creator.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "test.hpp"

namespace
{
using namespace ::testing;

iox::RouDiConfig_t createRouDiConfig(uint32_t chunksize, uint32_t numchunk)
{
    iox::mepoo::MePooConfig mempoolConfig;
    mempoolConfig.addMemPool({chunksize, numchunk});
    auto currentGroup = iox::posix::PosixGroup::getGroupOfCurrentProcess();
    iox::RouDiConfig_t roudiConfig;
    roudiConfig.m_sharedMemorySegments.push_back({currentGroup.getName(), currentGroup.getName(), mempoolConfig});
    return roudiConfig;
}

TEST(ShmCreatorDeathTest, AllocatingTooMuchMemoryLeadsToExitWithSIGBUS)
{
    // try a config with high memory requirements, expect failure
    auto badconfig = createRouDiConfig(1 << 30, 100);
    EXPECT_DEATH(iox::runtime::SharedMemoryCreator<iox::roudi::MiddlewareShm> sut(badconfig),
                 "\033\\[0;1;97;41mFatal error:\033\\[m the available memory is insufficient. Cannot allocate mempools "
                 "in shared memory. Please make sure that enough memory is available. For this, consider also the "
                 "memory which is required for the \\[/iceoryx_mgmt\\] segment. Please refer to "
                 "share\\/doc\\/iceoryx\\/FAQ.md in your release delivery.");
    auto goodconfig = createRouDiConfig(1024, 1);

    // try again with a config with low memory requirements; success clears shared memory allocated by the OS in e.g.
    // /dev/shm
    iox::runtime::SharedMemoryCreator<iox::roudi::MiddlewareShm> sut(goodconfig);
}
} // namespace