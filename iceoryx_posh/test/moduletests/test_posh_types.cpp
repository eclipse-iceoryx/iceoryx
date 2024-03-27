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

    EXPECT_THAT(iceoryxResourcePrefix(DEFAULT_DOMAIN_ID, ResourceType::ICEORYX_DEFINED).c_str(), StrEq("iox1_0_i_"));
}

TEST(PoshTypes_test, IceoryxResourcePrefixWithMaxDomainIdWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "049e79d7-d0ca-4951-8d44-c80aebab7a88");

    const char* EXPECTED_PREFIX = experimental::hasExperimentalPoshFeaturesEnabled() ? "iox1_65535_i_" : "iox1_0_i_";

    EXPECT_THAT(
        iceoryxResourcePrefix(DomainId{std::numeric_limits<uint16_t>::max()}, ResourceType::ICEORYX_DEFINED).c_str(),
        StrEq(EXPECTED_PREFIX));
}

TEST(PoshTypes_test, IceoryxResourcePrefixWithMaxDomainIdAndUserDefinedResourceTypeWorks)
{
    ::testing::Test::RecordProperty("TEST_ID", "b63bbdca-ff19-41bc-9f8a-c657b0ee8009");

    const char* EXPECTED_PREFIX = experimental::hasExperimentalPoshFeaturesEnabled() ? "iox1_65535_u_" : "iox1_0_u_";

    EXPECT_THAT(
        iceoryxResourcePrefix(DomainId{std::numeric_limits<uint16_t>::max()}, ResourceType::USER_DEFINED).c_str(),
        StrEq(EXPECTED_PREFIX));
}
