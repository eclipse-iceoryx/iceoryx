// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include "test.hpp"

using namespace ::testing;
using namespace iox;

TEST(PoshTypes_test, IceoryxResourcePrefixWithDefaultDomainIdWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "35f1d638-8efa-41dd-859b-bcc23450844f");

    const auto expected_prefix = iox::concatenate(IOX_DEFAULT_RESOURCE_PREFIX, "_0_i_");

    EXPECT_THAT(iceoryxResourcePrefix(DEFAULT_DOMAIN_ID, ResourceType::ICEORYX_DEFINED).c_str(),
                StrEq(expected_prefix.c_str()));
}

TEST(PoshTypes_test, IceoryxResourcePrefixWithMaxDomainIdWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "049e79d7-d0ca-4951-8d44-c80aebab7a88");

    constexpr uint64_t CAPACITY{100};
    char expected_prefix[CAPACITY];
    snprintf(expected_prefix,
             CAPACITY,
             "%s_%s_i_",
             IOX_DEFAULT_RESOURCE_PREFIX,
             experimental::hasExperimentalPoshFeaturesEnabled() ? "65535" : "0");
    expected_prefix[CAPACITY - 1] = 0;

    EXPECT_THAT(
        iceoryxResourcePrefix(DomainId{std::numeric_limits<uint16_t>::max()}, ResourceType::ICEORYX_DEFINED).c_str(),
        StrEq(expected_prefix));
}

TEST(PoshTypes_test, IceoryxResourcePrefixWithMaxDomainIdAndUserDefinedResourceTypeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b63bbdca-ff19-41bc-9f8a-c657b0ee8009");

    constexpr uint64_t CAPACITY{100};
    char expected_prefix[CAPACITY];
    snprintf(expected_prefix,
             CAPACITY,
             "%s_%s_u_",
             IOX_DEFAULT_RESOURCE_PREFIX,
             experimental::hasExperimentalPoshFeaturesEnabled() ? "65535" : "0");
    expected_prefix[CAPACITY - 1] = 0;

    EXPECT_THAT(
        iceoryxResourcePrefix(DomainId{std::numeric_limits<uint16_t>::max()}, ResourceType::USER_DEFINED).c_str(),
        StrEq(expected_prefix));
}
