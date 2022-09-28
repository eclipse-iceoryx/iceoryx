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

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_hoofs/testing/test_definitions.hpp"
#include "iceoryx_posh/error_handling/error_handling.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/segment_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/mepoo/segment_config.hpp"
#include "test.hpp"


namespace
{
using namespace ::testing;
using namespace iox::mepoo;
using namespace iox::posix;

class MePooSegmentMock
{
  public:
    MePooSegmentMock(const MePooConfig& mempoolConfig IOX_MAYBE_UNUSED,
                     Allocator& managementAllocator IOX_MAYBE_UNUSED,
                     const PosixGroup& readerGroup IOX_MAYBE_UNUSED,
                     const PosixGroup& writerGroup IOX_MAYBE_UNUSED,
                     const MemoryInfo& memoryInfo IOX_MAYBE_UNUSED) noexcept
    {
    }
};

class SegmentManager_test : public Test
{
  public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
        iox::rp::UntypedRelativePointer::unregisterAll();
    }

    MePooConfig getMempoolConfig()
    {
        MePooConfig config;
        config.addMemPool({128, 5});
        config.addMemPool({256, 7});
        return config;
    }

    SegmentConfig getSegmentConfig()
    {
        SegmentConfig config;
        config.m_sharedMemorySegments.push_back({"iox_roudi_test1", "iox_roudi_test2", mepooConfig});
        config.m_sharedMemorySegments.push_back({"iox_roudi_test2", "iox_roudi_test3", mepooConfig});
        return config;
    }

    SegmentConfig getInvalidSegmentConfig()
    {
        SegmentConfig config;
        config.m_sharedMemorySegments.push_back({"iox_roudi_test1", "iox_roudi_test1", mepooConfig});
        config.m_sharedMemorySegments.push_back({"iox_roudi_test3", "iox_roudi_test1", mepooConfig});
        return config;
    }

    SegmentConfig getSegmentConfigWithMaximumNumberOfSegements()
    {
        SegmentConfig config;
        for (uint64_t i = 0U; i < iox::MAX_SHM_SEGMENTS; ++i)
        {
            config.m_sharedMemorySegments.push_back({"iox_roudi_test1", "iox_roudi_test1", mepooConfig});
        }
        return config;
    }

    static constexpr size_t MEM_SIZE{20000};
    char memory[MEM_SIZE];
    iox::posix::Allocator allocator{memory, MEM_SIZE};
    MePooConfig mepooConfig = getMempoolConfig();
    SegmentConfig segmentConfig = getSegmentConfig();
    SegmentManager<> sut{segmentConfig, &allocator};
};

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getSegmentMappingsForReadUser))
{
    ::testing::Test::RecordProperty("TEST_ID", "a3af818a-c119-4182-948a-1e709c29d97f");
    auto mapping = sut.getSegmentMappings(PosixUser{"iox_roudi_test1"});
    ASSERT_THAT(mapping.size(), Eq(1u));
    EXPECT_THAT(mapping[0].m_isWritable, Eq(false));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getSegmentMappingsForWriteUser))
{
    ::testing::Test::RecordProperty("TEST_ID", "5e5d3128-5e4b-41e9-b541-ba9dc0bd57e3");
    auto mapping = sut.getSegmentMappings(PosixUser{"iox_roudi_test2"});
    ASSERT_THAT(mapping.size(), Eq(2u));
    EXPECT_THAT(mapping[0].m_isWritable == mapping[1].m_isWritable, Eq(false));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getSegmentMappingsEmptyForNonRegisteredUser))
{
    ::testing::Test::RecordProperty("TEST_ID", "7cf9a658-bb2d-444f-af67-0355e8f45ea2");
    auto mapping = sut.getSegmentMappings(PosixUser{"roudi_test4"});
    ASSERT_THAT(mapping.size(), Eq(0u));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getSegmentMappingsEmptyForNonExistingUser))
{
    ::testing::Test::RecordProperty("TEST_ID", "ca869fa8-49cd-43bc-8d72-4024b45f1f5f");
    auto mapping = sut.getSegmentMappings(PosixUser{"no_user"});
    ASSERT_THAT(mapping.size(), Eq(0u));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getMemoryManagerForUserWithWriteUser))
{
    ::testing::Test::RecordProperty("TEST_ID", "2fd4262a-20d2-4631-9b63-610944f28120");
    auto memoryManager = sut.getSegmentInformationWithWriteAccessForUser(PosixUser{"iox_roudi_test2"}).m_memoryManager;
    ASSERT_TRUE(memoryManager.has_value());
    ASSERT_THAT(memoryManager.value().get().getNumberOfMemPools(), Eq(2u));

    auto poolInfo1 = memoryManager.value().get().getMemPoolInfo(0);
    auto poolInfo2 = memoryManager.value().get().getMemPoolInfo(1);
    EXPECT_THAT(poolInfo1.m_numChunks, Eq(5u));
    EXPECT_THAT(poolInfo2.m_numChunks, Eq(7u));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getMemoryManagerForUserFailWithReadOnlyUser))
{
    ::testing::Test::RecordProperty("TEST_ID", "9d7c18fd-b8db-425a-830d-22c781091244");
    EXPECT_FALSE(
        sut.getSegmentInformationWithWriteAccessForUser(PosixUser{"iox_roudi_test1"}).m_memoryManager.has_value());
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getMemoryManagerForUserFailWithNonExistingUser))
{
    ::testing::Test::RecordProperty("TEST_ID", "bff18ab5-89ff-45e0-97ea-5409169ddf9a");
    EXPECT_FALSE(sut.getSegmentInformationWithWriteAccessForUser(PosixUser{"no_user"}).m_memoryManager.has_value());
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(addingMoreThanOneWriterGroupFails))
{
    ::testing::Test::RecordProperty("TEST_ID", "3fa29560-7341-43bf-a22e-2d3550b49e4e");
    SegmentConfig segmentConfig = getInvalidSegmentConfig();
    SegmentManager<> sut{segmentConfig, &allocator};

    iox::cxx::optional<iox::PoshError> detectedError;
    auto errorHandlerGuard = iox::ErrorHandlerMock::setTemporaryErrorHandler<iox::PoshError>(
        [&](const iox::PoshError error, const iox::ErrorLevel errorLevel) {
            detectedError.emplace(error);
            EXPECT_THAT(errorLevel, Eq(iox::ErrorLevel::FATAL));
        });

    sut.getSegmentMappings(PosixUser("iox_roudi_test1"));

    ASSERT_TRUE(detectedError.has_value());
    EXPECT_THAT(detectedError.value(), Eq(iox::PoshError::MEPOO__USER_WITH_MORE_THAN_ONE_WRITE_SEGMENT));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(addingMaximumNumberOfSegmentsWorks))
{
    ::testing::Test::RecordProperty("TEST_ID", "79db009a-da1a-4140-b375-f174af615d54");
    SegmentConfig segmentConfig = getSegmentConfigWithMaximumNumberOfSegements();
    SegmentManager<MePooSegmentMock> sut{segmentConfig, &allocator};
}

} // namespace
