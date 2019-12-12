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
#include "test_definitions.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/mepoo_segment.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"

#include <fcntl.h>
#include <functional>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>

using namespace ::testing;
using namespace iox::mepoo;

class MePooSegment_test : public Test
{
  public:
    struct SharedMemoryObject_MOCK
    {
        using createFct = std::function<void(const char*,
                                             const uint64_t,
                                             const iox::posix::AccessMode,
                                             const iox::posix::OwnerShip,
                                             const void*,
                                             const mode_t)>;
        static iox::cxx::optional<SharedMemoryObject_MOCK>
        create(const char* f_name,
               const uint64_t f_memorySizeInBytes,
               const iox::posix::AccessMode f_accessMode,
               const iox::posix::OwnerShip f_ownerShip,
               void* f_baseAddressHint,
               const mode_t f_permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
        {
            if (createVerificator)
            {
                createVerificator(
                    f_name, f_memorySizeInBytes, f_accessMode, f_ownerShip, f_baseAddressHint, f_permissions);
            }
            return SharedMemoryObject_MOCK(f_memorySizeInBytes, f_baseAddressHint);
        }

        SharedMemoryObject_MOCK(const uint64_t f_memorySizeInBytes, void* f_baseAddressHint)
            : m_memorySizeInBytes(f_memorySizeInBytes)
            , m_baseAddressHint(f_baseAddressHint)
        {
            filehandle = creat("/tmp/roudi_segment_test", S_IRWXU);
        }

        ~SharedMemoryObject_MOCK()
        {
            remove("/tmp/roudi_segment_test");
        }

        iox::posix::Allocator* getAllocator()
        {
            return allocator.get();
        }

        void finalizeAllocation()
        {
        }

        void* allocate(const uint64_t, const uint64_t = 0)
        {
            return nullptr;
        }

        int getFileHandle()
        {
            return filehandle;
        }

        uint64_t getSizeInBytes() const
        {
            return m_memorySizeInBytes;
        }

        void* getBaseAddress() const
        {
            return m_baseAddressHint;
        }

        uint64_t m_memorySizeInBytes{0};
        void* m_baseAddressHint{nullptr};
        static constexpr int MEM_SIZE = 100000;
        char memory[MEM_SIZE];
        std::shared_ptr<iox::posix::Allocator> allocator{new iox::posix::Allocator(memory, MEM_SIZE)};
        int filehandle;
        static createFct createVerificator;
    };

    void SetUp(){};
    void TearDown(){};

    MePooConfig setupMepooConfig()
    {
        MePooConfig config;
        config.addMemPool({128, 100});
        return config;
    }

    static constexpr uint64_t RawMemorySize{20000};
    uint8_t m_rawMemory[RawMemorySize];
    iox::posix::Allocator m_managementAllocator{iox::posix::Allocator(m_rawMemory, RawMemorySize)};

    MePooConfig mepooConfig = setupMepooConfig();
    MePooSegment<SharedMemoryObject_MOCK, MemoryManager> sut{
        mepooConfig, &m_managementAllocator, {"roudi_test1"}, {"roudi_test2"}, 0};
};
MePooSegment_test::SharedMemoryObject_MOCK::createFct MePooSegment_test::SharedMemoryObject_MOCK::createVerificator;

TEST_F(MePooSegment_test, DISABLED_sharedMemoryFileHandleRightsAfterConstructor)
{
    /// @todo
}

TEST_F(MePooSegment_test, ADD_TEST_WITH_ADDITIONAL_USER(SharedMemoryCreationParameter))
{
    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator = [](const char* f_name,
                                                                       const uint64_t,
                                                                       const iox::posix::AccessMode f_accessMode,
                                                                       const iox::posix::OwnerShip f_ownerShip,
                                                                       const void*,
                                                                       const mode_t) {
        EXPECT_THAT(std::string(f_name), Eq(std::string("/roudi_test2")));
        EXPECT_THAT(f_accessMode, Eq(iox::posix::AccessMode::readWrite));
        EXPECT_THAT(f_ownerShip, Eq(iox::posix::OwnerShip::mine));
    };
    MePooSegment<SharedMemoryObject_MOCK, MemoryManager> sut2{
        mepooConfig, &m_managementAllocator, {"roudi_test1"}, {"roudi_test2"}, 0};
    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator =
        MePooSegment_test::SharedMemoryObject_MOCK::createFct();
}

TEST_F(MePooSegment_test, ADD_TEST_WITH_ADDITIONAL_USER(GetSharedMemoryObject))
{
    uint64_t memorySizeInBytes{0};
    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator = [&](const char*,
                                                                        const uint64_t f_memorySizeInBytes,
                                                                        const iox::posix::AccessMode,
                                                                        const iox::posix::OwnerShip,
                                                                        const void*,
                                                                        const mode_t) {
        memorySizeInBytes = f_memorySizeInBytes;
    };
    MePooSegment<SharedMemoryObject_MOCK, MemoryManager> sut2{
        mepooConfig, &m_managementAllocator, {"roudi_test1"}, {"roudi_test2"}, 0};
    MePooSegment_test::SharedMemoryObject_MOCK::createVerificator =
        MePooSegment_test::SharedMemoryObject_MOCK::createFct();

    EXPECT_THAT(sut2.getSharedMemoryObject().getSizeInBytes(), Eq(memorySizeInBytes));
}

TEST_F(MePooSegment_test, ADD_TEST_WITH_ADDITIONAL_USER(GetReaderGroup))
{
    EXPECT_THAT(sut.getReaderGroup(), Eq(iox::posix::PosixGroup("roudi_test1")));
}

TEST_F(MePooSegment_test, ADD_TEST_WITH_ADDITIONAL_USER(GetWriterGroup))
{
    EXPECT_THAT(sut.getWriterGroup(), Eq(iox::posix::PosixGroup("roudi_test2")));
}

TEST_F(MePooSegment_test, ADD_TEST_WITH_ADDITIONAL_USER(GetMemoryManager))
{
    ASSERT_THAT(sut.getMemoryManager().getNumberOfMemPools(), Eq(1));
    auto config = sut.getMemoryManager().getMemPoolInfo(0);
    ASSERT_THAT(config.m_numChunks, Eq(100u));
    auto chunk = sut.getMemoryManager().getChunk(128);
    EXPECT_THAT(chunk.getChunkHeader()->m_info.m_payloadSize, Eq(128u));
}
