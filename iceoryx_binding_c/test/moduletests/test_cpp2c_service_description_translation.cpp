// Copyright (c) 2021 Apex.AI Inc. All rights reserved.
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

#include "iceoryx_binding_c/internal/cpp2c_service_description_translation.hpp"

#include "test.hpp"

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::capro;

TEST(iox_service_description_translation_test, TranslatesStringCorrectly)
{
    ::testing::Test::RecordProperty("TEST_ID", "0f3c056e-0f3c-4266-85dd-6d9021616c16");
    iox::capro::ServiceDescription service(
        IdString_t("SomeService"), IdString_t("FunkyInstance"), IdString_t("BumbleBeeSighted"));
    auto cServiceDescription = TranslateServiceDescription(service);

    EXPECT_THAT(std::string(cServiceDescription.serviceString), Eq("SomeService"));
    EXPECT_THAT(std::string(cServiceDescription.instanceString), Eq("FunkyInstance"));
    EXPECT_THAT(std::string(cServiceDescription.eventString), Eq("BumbleBeeSighted"));
}

} // namespace
