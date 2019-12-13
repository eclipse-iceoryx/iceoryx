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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/segment_manager.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/mepoo/segment_config.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"
#include "test.hpp"
#include "test_definitions.hpp"


using namespace ::testing;
using namespace iox::mepoo;

class SegmentManager_test : public Test
{
  public:
    SegmentManager_test()
        : surpressOutput{SurpressOutput(true)}
    {
    }

    ~SegmentManager_test()
    {
        if (surpressOutput)
        {
            if (Test::HasFailure())
            {
                std::cout << testing::internal::GetCapturedStderr() << std::endl;
            }
            else
            {
                (void)testing::internal::GetCapturedStderr();
            }
        }
    }

    void SetUp(){};
    void TearDown()
    {
        iox::RelativePointer::unregisterAll();
    };

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
        segmentConfig.m_sharedMemorySegments.push_back({"roudi_test1", "roudi_test2", mepooConfig});
        segmentConfig.m_sharedMemorySegments.push_back({"roudi_test2", "roudi_test3", mepooConfig});
        return config;
    }

    bool SurpressOutput(const bool v)
    {
        if (v)
        {
            testing::internal::CaptureStderr();
        }
        return v;
    }

    bool surpressOutput;
    static constexpr size_t MEM_SIZE{20000};
    char memory[MEM_SIZE];
    iox::posix::Allocator allocator{memory, MEM_SIZE};
    MePooConfig mepooConfig = getMempoolConfig();
    SegmentConfig segmentConfig = getSegmentConfig();
    SegmentManager<> sut{segmentConfig, &allocator, 0, false};
};

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getSegmentMappingsForReadUser))
{
    auto mapping = sut.getSegmentMappings({"roudi_test1"});
    ASSERT_THAT(mapping.size(), Eq(1));
    EXPECT_THAT(mapping[0].m_isWritable, Eq(false));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getSegmentMappingsForWriteUser))
{
    auto mapping = sut.getSegmentMappings({"roudi_test2"});
    ASSERT_THAT(mapping.size(), Eq(2));
    EXPECT_THAT(mapping[0].m_isWritable == mapping[1].m_isWritable, Eq(false));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getSegmentMappingsEmptyForNonRegisteredUser))
{
    auto mapping = sut.getSegmentMappings({"roudi_test4"});
    ASSERT_THAT(mapping.size(), Eq(0));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getSegmentMappingsEmptyForNonExistingUser))
{
    auto mapping = sut.getSegmentMappings({"no_user"});
    ASSERT_THAT(mapping.size(), Eq(0));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getMemoryManagerForUserWithWriteUser))
{
    auto memoryManager = sut.getSegmentInformationForUser({"roudi_test2"}).m_memoryManager;
    ASSERT_THAT(memoryManager, Ne(nullptr));
    ASSERT_THAT(memoryManager->getNumberOfMemPools(), Eq(2));

    auto poolInfo1 = memoryManager->getMemPoolInfo(0);
    auto poolInfo2 = memoryManager->getMemPoolInfo(1);
    EXPECT_THAT(poolInfo1.m_numChunks, Eq(5u));
    EXPECT_THAT(poolInfo2.m_numChunks, Eq(7u));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getMemoryManagerForUserFailWithReadOnlyUser))
{
    EXPECT_THAT(sut.getSegmentInformationForUser({"roudi_test1"}).m_memoryManager, Eq(nullptr));
}

TEST_F(SegmentManager_test, ADD_TEST_WITH_ADDITIONAL_USER(getMemoryManagerForUserFailWithNonExistingUser))
{
    EXPECT_THAT(sut.getSegmentInformationForUser({"no_user"}).m_memoryManager, Eq(nullptr));
}
