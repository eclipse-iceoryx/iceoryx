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

#include "iceoryx_posh/runtime/service_discovery.hpp"
#include "iceoryx_posh/testing/roudi_gtest.hpp"

using namespace iox;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/service_discovery.h"
}

namespace
{
using namespace ::testing;

class iox_service_discovery_test : public RouDi_GTest
{
  public:
    void SetUp() override
    {
        iox_runtime_init(runtimeName.c_str());
        sut = iox_service_discovery_init(&sutStorage);
    }

    void TearDown() override
    {
        iox_service_discovery_deinit(sut);
    }

    std::string runtimeName = "runtime";
    iox_service_discovery_storage_t sutStorage;
    iox_service_discovery_t sut;

    static cxx::vector<iox_service_description_t, MAX_FINDSERVICE_RESULT_SIZE> searchResult;
    static void findHandler(const iox_service_description_t s)
    {
        searchResult.emplace_back(s);
    }
};

cxx::vector<iox_service_description_t, MAX_FINDSERVICE_RESULT_SIZE> iox_service_discovery_test::searchResult;


TEST_F(iox_service_discovery_test, FindServiceWithNullptrsForServiceInstanceEventsReturnsAllServices)
{
    iox_service_discovery_find_service(sut, nullptr, nullptr, nullptr, findHandler);
    for (const auto& service : searchResult)
    {
        EXPECT_THAT(service.instanceString, StrEq("RouDi_ID"));
    }
}

} // namespace
