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

#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include "test.hpp"

#include <cstring>
#include <fcntl.h> /* For O_* constants */
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
using namespace ::testing;

namespace
{
constexpr mode_t ShmMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
constexpr int OFlags = O_CREAT | O_RDWR;
constexpr size_t ShmSize = 4096 * 32;
constexpr uint32_t BaseAddress1 = 0x1F400000;
constexpr uint32_t BaseAddress2 = 0x3E800000;

struct Data
{
    Data(uint32_t i, uint32_t j)
        : Data1(i)
        , Data2(j)
    {
    }
    uint32_t Data1 = 27;
    uint32_t Data2 = 72;
};

template <size_t n>
class Memory
{
  public:
    Memory()
    {
    }

    std::uint8_t* operator[](int i)
    {
        return &buf[i];
    }

    std::uint8_t buf[n];
};

class RelativePointer_test : public Test
{
  public:
    void SetUp() override
    {
        m_fileDescriptor = shm_open("TestShm", OFlags, ShmMode);
        ftruncate(m_fileDescriptor, ShmSize);
        internal::CaptureStderr();
    }

    void TearDown() override
    {
        shm_unlink("TestShm");
        iox::RelativePointer::unregisterAll();
        std::string output = internal::GetCapturedStderr();
        if (Test::HasFailure())
        {
            std::cout << output << std::endl;
        }
    }
    int m_fileDescriptor;
};

template <typename T>
class relativeptrtests : public RelativePointer_test
{
};
class MemMap
{
  public:
    MemMap(void* addr, int fileDescriptor)
    {
        m_mapAddr = mmap(addr, ShmSize, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, 0);
        EXPECT_EQ(addr, m_mapAddr);
    }
    ~MemMap()
    {
        munmap(m_mapAddr, ShmSize);
        m_mapAddr = nullptr;
    }

    void* getMappedAddress()
    {
        return m_mapAddr;
    }

  private:
    void* m_mapAddr = nullptr;
};

typedef testing::Types<uint8_t, int8_t, double> Types;
TYPED_TEST_CASE(relativeptrtests, Types);


TYPED_TEST(relativeptrtests, ConstrTests)
{
    void* basePtr1 = reinterpret_cast<void*>(BaseAddress1);
    MemMap memMap(basePtr1, this->m_fileDescriptor);
    void* basePtr2 = reinterpret_cast<void*>(BaseAddress2);
    MemMap memMap1(basePtr2, this->m_fileDescriptor);

    EXPECT_EQ(iox::RelativePointer::registerPtr(1, memMap.getMappedAddress(), ShmSize), true);
    EXPECT_EQ(iox::RelativePointer::registerPtr(2, memMap1.getMappedAddress(), ShmSize), true);

    {
        auto offset = ShmSize / 2;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        iox::relative_ptr<TypeParam> rp(memMap.getMappedAddress());
        EXPECT_EQ(rp.getOffset(), 0);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = ShmSize / 2;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp(adr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = ShmSize - 1;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp(adr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        iox::relative_ptr<TypeParam> rp(memMap1.getMappedAddress());
        EXPECT_EQ(rp.getOffset(), 0);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = ShmSize / 2;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap1.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp(adr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = ShmSize - 1;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap1.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp(adr);
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        iox::relative_ptr<TypeParam> rp(nullptr);
        EXPECT_EQ(rp, nullptr);
    }

    {
        auto offset = ShmSize + 1;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap1.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp(adr);
        EXPECT_NE(rp, nullptr);
    }
}

TYPED_TEST(relativeptrtests, AssignmentOperatorTests)
{
    void* basePtr1 = reinterpret_cast<void*>(BaseAddress1);
    MemMap memMap(basePtr1, this->m_fileDescriptor);
    void* basePtr2 = reinterpret_cast<void*>(BaseAddress2);
    MemMap memMap1(basePtr2, this->m_fileDescriptor);

    EXPECT_EQ(iox::RelativePointer::registerPtr(1, memMap.getMappedAddress(), ShmSize), true);
    EXPECT_EQ(iox::RelativePointer::registerPtr(2, memMap1.getMappedAddress(), ShmSize), true);

    {
        iox::relative_ptr<TypeParam> rp;
        rp = memMap.getMappedAddress();
        EXPECT_EQ(rp.getOffset(), 0);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = ShmSize / 2;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = ShmSize - 1;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 1);
        EXPECT_NE(rp, nullptr);
    }

    {
        iox::relative_ptr<TypeParam> rp;
        rp = memMap1.getMappedAddress();
        EXPECT_EQ(rp.getOffset(), 0);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = ShmSize / 2;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap1.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        auto offset = ShmSize - 1;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap1.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp;
        rp = adr;
        EXPECT_EQ(rp.getOffset(), offset);
        EXPECT_EQ(rp.getId(), 2);
        EXPECT_NE(rp, nullptr);
    }

    {
        iox::relative_ptr<TypeParam> rp;
        rp = nullptr;
        EXPECT_EQ(rp, nullptr);
    }

    {
        auto offset = ShmSize + 1;
        void* adr = reinterpret_cast<void*>(reinterpret_cast<size_t>(memMap1.getMappedAddress() + offset));
        iox::relative_ptr<TypeParam> rp;
        rp = adr;
        EXPECT_NE(rp, nullptr);
    }
}

TYPED_TEST(relativeptrtests, IdAndOffset)
{
    void* basePtr1 = reinterpret_cast<void*>(BaseAddress1);
    MemMap memMap(basePtr1, this->m_fileDescriptor);

    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);
    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
    EXPECT_EQ(rp1.getOffset(), reinterpret_cast<std::ptrdiff_t>(basePtr1));
    EXPECT_EQ(rp1.getId(), 1);

    int offset = ShmSize / 2;
    auto offsetAddr1 = reinterpret_cast<TypeParam*>(memMap.getMappedAddress() + offset);
    iox::relative_ptr<TypeParam> rp2(offsetAddr1, 1);
    EXPECT_EQ(rp2.getOffset(), offset);
    EXPECT_EQ(rp2.getId(), 1);
    EXPECT_EQ(rp2.get(), offsetAddr1);
}

TYPED_TEST(relativeptrtests, getOffset)
{
    void* basePtr = reinterpret_cast<void*>(BaseAddress1);
    MemMap memMap(basePtr, this->m_fileDescriptor);

    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);
    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
    EXPECT_EQ(iox::RelativePointer::getOffset(1, memMap.getMappedAddress()), 0);

    int offset = ShmSize / 2;
    auto offsetAddr1 = reinterpret_cast<int*>(memMap.getMappedAddress() + offset);
    iox::relative_ptr<TypeParam> rp2(offsetAddr1, 1);
    EXPECT_EQ(iox::RelativePointer::getOffset(1, offsetAddr1), offset);
}

TYPED_TEST(relativeptrtests, getPtr)
{
    void* basePtr = reinterpret_cast<void*>(BaseAddress1);
    MemMap memMap(basePtr, this->m_fileDescriptor);

    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);
    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
    EXPECT_EQ(iox::RelativePointer::getPtr(1, 0), memMap.getMappedAddress());

    int offset = ShmSize / 2;
    auto offsetAddr1 = reinterpret_cast<int*>(memMap.getMappedAddress() + offset);
    iox::relative_ptr<TypeParam> rp2(offsetAddr1, 1);
    EXPECT_EQ(iox::RelativePointer::getPtr(1, offset), offsetAddr1);
}

TYPED_TEST(relativeptrtests, registerPtr)
{
    MemMap memMap(reinterpret_cast<void*>(BaseAddress1), this->m_fileDescriptor);
    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);

    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), false);
    EXPECT_EQ(rp1.unregisterPtr(1), true);
    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
}

TYPED_TEST(relativeptrtests, unRegisterPointerTest_Valid)
{
    MemMap memMap(reinterpret_cast<void*>(BaseAddress1), this->m_fileDescriptor);
    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);

    rp1.registerPtr(1, memMap.getMappedAddress());
    EXPECT_EQ(rp1.unregisterPtr(1), true);
    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
}

TYPED_TEST(relativeptrtests, unregisterPointerAll)
{
    MemMap memMap(reinterpret_cast<void*>(BaseAddress1), this->m_fileDescriptor);
    MemMap memMap1(reinterpret_cast<void*>(BaseAddress2), this->m_fileDescriptor);

    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);
    iox::relative_ptr<TypeParam> rp2(memMap1.getMappedAddress(), 9999);

    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
    EXPECT_EQ(rp2.registerPtr(9999, memMap1.getMappedAddress()), true);
    iox::RelativePointer::unregisterAll();
    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
    EXPECT_EQ(rp2.registerPtr(9999, memMap1.getMappedAddress()), true);
}

TYPED_TEST(relativeptrtests, registerPtrWithId)
{
    MemMap memMap(reinterpret_cast<void*>(BaseAddress1), this->m_fileDescriptor);
    MemMap memMap1(reinterpret_cast<void*>(BaseAddress2), this->m_fileDescriptor);

    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);
    iox::relative_ptr<TypeParam> rp2(memMap1.getMappedAddress(), 10000);

    EXPECT_EQ(rp1.registerPtr(1, memMap.getMappedAddress()), true);
    EXPECT_EQ(rp2.registerPtr(10000, memMap1.getMappedAddress()), false);

    // ToCheck can same memory be registered to different id?
    // EXPECT_EQ(rp1.registerPtr(2, memMap.getMappedAddress()), false);
    EXPECT_EQ(rp2.registerPtr(10000, memMap1.getMappedAddress()), false);
}

TYPED_TEST(relativeptrtests, basePointerValid)
{
    void* basePtr1 = reinterpret_cast<void*>(BaseAddress1);
    MemMap memMap(basePtr1, this->m_fileDescriptor);

    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);
    EXPECT_EQ(nullptr, rp1.getBasePtr(1));
    rp1.registerPtr(1, memMap.getMappedAddress());
    EXPECT_EQ(basePtr1, rp1.getBasePtr(1));
}

TYPED_TEST(relativeptrtests, assignmentOperator)
{
    MemMap memMap(reinterpret_cast<void*>(BaseAddress1), this->m_fileDescriptor);

    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);
    iox::relative_ptr<TypeParam> rp2 = rp1;

    EXPECT_EQ(rp1.getBasePtr(), rp2.getBasePtr());
    EXPECT_EQ(rp1.getId(), rp2.getId());
    EXPECT_EQ(rp1.getOffset(), rp2.getOffset());
}

TYPED_TEST(relativeptrtests, pointerOperator)
{
    MemMap memMap(reinterpret_cast<void*>(BaseAddress1), this->m_fileDescriptor);

    auto baseAddr = reinterpret_cast<TypeParam*>(memMap.getMappedAddress());
    *baseAddr = static_cast<TypeParam>(88);
    iox::relative_ptr<TypeParam> rp1(memMap.getMappedAddress(), 1);

    EXPECT_EQ(*rp1, *baseAddr);
    *baseAddr = static_cast<TypeParam>(99);
    EXPECT_EQ(*rp1, *baseAddr);
}

// central use case of the relative pointer:
// it is tested that changing the (static) lookup table of a relative pointer causes existing
// relative pointers point to changed locations relative to the new lookup table
TEST_F(RelativePointer_test, memoryRemapping)
{
    constexpr size_t BLOCK_SIZE = 1024;
    // simulate 3 consecutive memory blocks on the stack
    Memory<BLOCK_SIZE> block1;
    Memory<BLOCK_SIZE> block2;

    uint8_t* base1 = block1[0];
    uint8_t* base2 = block2[0];

    // compiler may rearrange data on stack so this can fail (but does not matter)
    // EXPECT_EQ(base2 - base1, BLOCK_SIZE);

    // uint8 write
    *base1 = 37U;
    *base2 = 73U;

    EXPECT_EQ(*base1, 37U);
    EXPECT_EQ(*base2, 73U);

    int offset = BLOCK_SIZE / 2;
    auto adr1 = reinterpret_cast<int*>(base1 + offset);
    auto adr2 = reinterpret_cast<int*>(base2 + offset);

    // int write
    *adr1 = 12;
    *adr2 = 21;

    EXPECT_EQ(*adr1, 12);
    EXPECT_EQ(*adr2, 21);

    EXPECT_EQ(iox::RelativePointer::registerPtr(1, base1), true);
    EXPECT_EQ(iox::RelativePointer::registerPtr(2, base2), true);

    {
        // the relative pointers point to base 1 and base 2l
        iox::relative_ptr<uint8_t> rp1(base1, 1);
        iox::relative_ptr<uint8_t> rp2(base2, 2);

        EXPECT_EQ(rp1.getId(), 1);
        EXPECT_EQ(rp2.getId(), 2);

        EXPECT_EQ(rp1.getOffset(), 0);
        EXPECT_EQ(rp2.getOffset(), 0);

        EXPECT_EQ(*rp1, 37U);
        EXPECT_EQ(*rp2, 73U);
    }

    {
        // now test with a type that is larger than 1 byte
        iox::relative_ptr<int> rp1(adr1, 1);
        iox::relative_ptr<int> rp2(adr2, 2);

        EXPECT_EQ(rp1.getId(), 1);
        EXPECT_EQ(rp2.getId(), 2);

        // relative to their respective memory block base adress both have the same offset
        EXPECT_EQ(rp1.getOffset(), offset);
        EXPECT_EQ(rp2.getOffset(), offset);

        //*** rp1 points to 12 and rp2 to 21
        EXPECT_EQ(*rp1, 12);
        EXPECT_EQ(*rp2, 21);

        // simulate a remapping, index 1 now refers to base 2 and vice versa ...
        EXPECT_EQ(iox::RelativePointer::unregisterPtr(1), true);
        EXPECT_EQ(iox::RelativePointer::unregisterPtr(2), true);

        EXPECT_EQ(iox::RelativePointer::registerPtr(1, base2), true);
        EXPECT_EQ(iox::RelativePointer::registerPtr(2, base1), true);

        // which, despite the relative pointer objects not having changed themselves,
        // leads to them referencing the respective other value now (compared to *** above)
        EXPECT_EQ(*rp1, 21);
        EXPECT_EQ(*rp2, 12);

        // this would also happen in another application where the static base pointer lookup table
        // is might differ from application to application
    }
}

TEST_F(RelativePointer_test, MemoryReMapping_SharedMemory)
{
    MemMap memMapWriter(reinterpret_cast<void*>(BaseAddress1), this->m_fileDescriptor);
    MemMap memMapReader(reinterpret_cast<void*>(BaseAddress2), this->m_fileDescriptor);

    Data* dataPointer1 = new (reinterpret_cast<void*>(BaseAddress1)) Data(12, 21);

    EXPECT_EQ(reinterpret_cast<uintptr_t>(dataPointer1), BaseAddress1);
    EXPECT_EQ(dataPointer1->Data1, reinterpret_cast<Data*>(BaseAddress2)->Data1);

    int offset = ShmSize / 2;
    auto offsetAddr1 = reinterpret_cast<int*>(memMapWriter.getMappedAddress() + offset);
    auto offsetAddr2 = reinterpret_cast<int*>(memMapReader.getMappedAddress() + offset);
    *offsetAddr1 = 37;

    EXPECT_EQ(*offsetAddr2, *offsetAddr1);

    EXPECT_EQ(iox::RelativePointer::registerPtr(1, reinterpret_cast<void*>(memMapWriter.getMappedAddress())), true);
    {
        iox::relative_ptr<int> rp1(offsetAddr1, 1);

        EXPECT_EQ(rp1.getId(), 1);
        EXPECT_EQ(rp1.getOffset(), offset);
        EXPECT_EQ(*rp1, 37);
        EXPECT_EQ(rp1.get(), offsetAddr1);
    }
    EXPECT_EQ(iox::RelativePointer::unregisterPtr(1), true);

    EXPECT_EQ(iox::RelativePointer::registerPtr(1, reinterpret_cast<void*>(memMapReader.getMappedAddress())), true);
    {
        iox::relative_ptr<int> rp1(offsetAddr2, 1);

        EXPECT_EQ(rp1.getId(), 1);
        EXPECT_EQ(rp1.getOffset(), offset);
        EXPECT_EQ(*rp1, 37);
        EXPECT_EQ(rp1.get(), offsetAddr2);
    }
    EXPECT_EQ(iox::RelativePointer::unregisterPtr(1), true);
}
} // namespace
