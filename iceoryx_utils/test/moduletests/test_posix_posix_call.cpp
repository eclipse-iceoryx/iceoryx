// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/posix_wrapper/posix_call.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::posix;

namespace
{
int testFunction(int a, int b)
{
    errno = a * b;
    return a + b;
}

class PosixCall_test : public Test
{
  public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }
};
} // namespace

TEST_F(PosixCall_test, callTest)
{
    posixCall(testFunction).call(1, 2).successReturnValue(4).evaluate().and_then([](auto& r) {}).or_else([](auto& r) {
    });
}

