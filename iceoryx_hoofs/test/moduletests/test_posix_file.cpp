// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/posix_wrapper/thread.hpp"
#include "iceoryx_hoofs/testing/barrier.hpp"
#include "iceoryx_platform/platform_settings.hpp"
#include "iox/duration.hpp"
#include "test.hpp"

#include <thread>

namespace
{
using namespace ::testing;
using namespace iox::posix;
using namespace iox::cxx;
using namespace iox;
using namespace iox::units;
using namespace iox::units::duration_literals;

struct File_test : public Test
{
};

TEST_F(File_test, CreateThreadWithNonEmptyCallableSucceeds)
{
    ::testing::Test::RecordProperty("TEST_ID", "f11e3aae-2e63-468f-b58a-22aeeedbd7fc");
}

} // namespace
