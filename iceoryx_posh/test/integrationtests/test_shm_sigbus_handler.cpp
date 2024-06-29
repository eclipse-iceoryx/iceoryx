// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#if !defined(__APPLE__)
#include "iceoryx_posh/iceoryx_posh_config.hpp"
#include "iceoryx_posh/internal/roudi/memory/mempool_collection_memory_block.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"
#include "test.hpp"

#include <thread>
namespace
{
using namespace ::testing;

TEST(ShmCreatorDeathTest, AllocatingTooMuchMemoryLeadsToExitWithSIGBUS)
{
    ::testing::Test::RecordProperty("TEST_ID", "d6c8949d-42c9-4b2c-a150-4306cb2a57f6");

#if (defined(unix) | defined(__unix) | defined(__unix__))
    GTEST_SKIP() << "This test does not run reliable on the CI on unix like systems";
#endif

    const iox::ShmName_t TEST_SHM_NAME{ "test_name" };
    // the death test makes only sense on platforms which are zeroing the whole shared memory
    // if the memory is only reserved a death will never occur
    if (iox::platform::IOX_SHM_WRITE_ZEROS_ON_CREATION)
    {
        // try a config with high memory requirements, expect failure
        iox::mepoo::MePooConfig badconfig;
        badconfig.addMemPool({ 1 << 30, 100 });
        iox::roudi::MemPoolCollectionMemoryBlock badmempools(badconfig);
        iox::roudi::PosixShmMemoryProvider badShmProvider(
            TEST_SHM_NAME, iox::DEFAULT_DOMAIN_ID, iox::AccessMode::READ_WRITE, iox::OpenMode::PURGE_AND_CREATE);
        ASSERT_FALSE(badShmProvider.addMemoryBlock(&badmempools).has_error());

        GTEST_FLAG(death_test_style) = "threadsafe";
        EXPECT_DEATH(IOX_DISCARD_RESULT(badShmProvider.create()), ".*");
    }

    // try again with a config with low memory requirements; success clears shared memory allocated by the OS in
    // e.g. /dev/shm
    iox::mepoo::MePooConfig goodconfig;
    goodconfig.addMemPool({ 1024, 1 });
    iox::roudi::MemPoolCollectionMemoryBlock goodmempools(goodconfig);
    iox::roudi::PosixShmMemoryProvider goodShmProvider(
        TEST_SHM_NAME, iox::DEFAULT_DOMAIN_ID, iox::AccessMode::READ_WRITE, iox::OpenMode::PURGE_AND_CREATE);
    ASSERT_FALSE(goodShmProvider.addMemoryBlock(&goodmempools).has_error());
    ASSERT_FALSE(goodShmProvider.create().has_error());
}
} // namespace

#endif
