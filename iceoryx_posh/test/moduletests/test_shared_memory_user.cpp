// Copyright (c) 2019, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/runtime/shared_memory_user.hpp"
#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"
#include "iceoryx_posh/internal/roudi_environment/roudi_environment.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox::runtime;
using namespace iox::roudi;

const bool doMapSharedMemoryIntoThread{"TRUE"};
const size_t topicSize{1U};
const uint64_t segmentId{1U};
iox::RelativePointer::offset_t segmentManagerAddressOffset{0U};

namespace iox
{
namespace test
{
class shared_memory_user_test : public Test
{
   public:
    shared_memory_user_test()
    {
    }

    virtual ~shared_memory_user_test()
    {
    }

    virtual void SetUp()
    {
        SharedMemoryUser ShmUser(doMapSharedMemoryIntoThread, topicSize, segmentId, segmentManagerAddressOffset);
    };

    virtual void TearDown()
    {
    };
    const ProcessName_t m_runtimeName{"App"};
    RouDiEnvironment m_roudiEnv{iox::RouDiConfig_t().setDefaults()};
    PoshRuntime* m_runtime{&iox::runtime::PoshRuntime::initRuntime(m_runtimeName)}; 
};

TEST_F(shared_memory_user_test, ConstructorShmObjectIsSuccess)
{
    auto sut = iox::posix::SharedMemoryObject::create(
       "/validShmMem", 100, iox::posix::AccessMode::readWrite, iox::posix::OwnerShip::mine, nullptr);
    EXPECT_THAT(sut.has_value(), Eq(true));  
}
} // namespace test
} // namespace iox