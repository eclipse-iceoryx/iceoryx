// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/watch_dog.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_roudi.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_posh/popo/untyped_client.hpp"
#include "iceoryx_posh/testing/mocks/posh_runtime_mock.hpp"

using namespace iox::popo;
using namespace iox::capro;
using namespace iox::capro;
using namespace iox::cxx;

extern "C" {
#include "iceoryx_binding_c/client.h"
}

#include "test.hpp"

namespace
{
using namespace ::testing;

class iox_client_test : public Test
{
  public:
    static constexpr const char RUNTIME_NAME[] = "spongebob_floodler";

    std::unique_ptr<PoshRuntimeMock> runtimeMock = PoshRuntimeMock::create(RUNTIME_NAME);

    void SetUp() override
    {
        memoryConfig.addMemPool({1024, 100});
        memoryManager.configureMemoryManager(memoryConfig, mgmtAllocator, dataAllocator);
    }

    ClientPortData* createClientPortData(const ClientOptions& options)
    {
        sutPort.emplace(ServiceDescription{IdString_t(TruncateToCapacity, SERVICE),
                                           IdString_t(TruncateToCapacity, INSTANCE),
                                           IdString_t(TruncateToCapacity, EVENT)},
                        RUNTIME_NAME,
                        options,
                        &memoryManager);
        return &*sutPort;
    }

    static constexpr uint64_t MANAGEMENT_MEMORY_SIZE = 1024 * 1024;
    char managmentMemory[MANAGEMENT_MEMORY_SIZE];
    iox::posix::Allocator mgmtAllocator{managmentMemory, MANAGEMENT_MEMORY_SIZE};
    static constexpr uint64_t DATA_MEMORY_SIZE = 1024 * 1024;
    char dataMemory[DATA_MEMORY_SIZE];
    iox::posix::Allocator dataAllocator{dataMemory, DATA_MEMORY_SIZE};
    iox::mepoo::MemoryManager memoryManager;
    iox::mepoo::MePooConfig memoryConfig;
    iox::cxx::optional<ClientPortData> sutPort;

    static constexpr const char SERVICE[] = "allGlory";
    static constexpr const char INSTANCE[] = "ToThe";
    static constexpr const char EVENT[] = "HYPNOTOAD";
};
constexpr const char iox_client_test::RUNTIME_NAME[];
constexpr const char iox_client_test::SERVICE[];
constexpr const char iox_client_test::INSTANCE[];
constexpr const char iox_client_test::EVENT[];

TEST_F(iox_client_test, notInitializedOptionsAreUninitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "d2fe0029-733f-4890-8624-b643a9aa3353");
    iox_client_options_t uninitializedOptions;
    // ignore the warning since we would like to test the behavior of an uninitialized option
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    EXPECT_FALSE(iox_client_options_is_initialized(&uninitializedOptions));
#pragma GCC diagnostic pop
}

TEST_F(iox_client_test, initializedOptionsAreInitialized)
{
    ::testing::Test::RecordProperty("TEST_ID", "906f01bd-2db3-466d-b785-d1cefd9a755a");
    iox_client_options_t initializedOptions;
    iox_client_options_init(&initializedOptions);
    EXPECT_TRUE(iox_client_options_is_initialized(&initializedOptions));
}

TEST_F(iox_client_test, InitialConnectionStateOnPortWithConnectOnCreateIs_CONNECTED)
{
    ::testing::Test::RecordProperty("TEST_ID", "1317bd7a-c6fe-445f-be44-502177ec885c");
    iox_client_storage_t sutStorage;
    ClientOptions defaultOptions;
    EXPECT_CALL(*runtimeMock,
                getMiddlewareClient(ServiceDescription{IdString_t(TruncateToCapacity, SERVICE),
                                                       IdString_t(TruncateToCapacity, INSTANCE),
                                                       IdString_t(TruncateToCapacity, EVENT)},
                                    defaultOptions,
                                    _))
        .WillOnce(Return(createClientPortData(defaultOptions)));


    iox_client_t sut = iox_client_init(&sutStorage, SERVICE, INSTANCE, EVENT, nullptr);

    ASSERT_THAT(sut, Ne(nullptr));
    EXPECT_THAT(iox_client_get_connection_state(sut), Eq(ConnectionState_CONNECTED));
}
} // namespace
