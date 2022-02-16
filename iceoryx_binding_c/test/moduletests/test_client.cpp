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
  protected:
    iox_client_test()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }
};


TEST_F(iox_client_test, initSubscriberWithNullptrForStorageReturnsNullptr)
{
}
} // namespace
