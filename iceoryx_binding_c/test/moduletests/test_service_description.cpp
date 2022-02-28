// Copyright (c) 2021 - 2022 Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/capro/service_description.hpp"

extern "C" {
#include "iceoryx_binding_c/service_description.h"
}

#include "test.hpp"

#include <type_traits>

namespace
{
using namespace ::testing;
using namespace iox;
using namespace iox::capro;

TEST(iox_service_description_test, StringSizesAreCorrect)
{
    constexpr uint64_t ZERO_TERMINATOR_SIZE = 1U;
    ::testing::Test::RecordProperty("TEST_ID", "f32a6d19-9ff7-4913-a1d8-39d46267b114");
    EXPECT_THAT(sizeof(decltype(std::declval<iox_service_description_t>().serviceString)),
                Eq(iox::capro::IdString_t().capacity() + ZERO_TERMINATOR_SIZE));
    EXPECT_THAT(sizeof(decltype(std::declval<iox_service_description_t>().instanceString)),
                Eq(iox::capro::IdString_t().capacity() + ZERO_TERMINATOR_SIZE));
    EXPECT_THAT(sizeof(decltype(std::declval<iox_service_description_t>().eventString)),
                Eq(iox::capro::IdString_t().capacity() + ZERO_TERMINATOR_SIZE));
}

} // namespace
