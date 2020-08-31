// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "test.hpp"

#include <iostream>

using namespace ::testing;

class PublisherSampleTest : public Test {

public:
    PublisherSampleTest()
    {

    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

};

TEST_F(PublisherSampleTest, CanHaveSamplesOfPrimitives)
{
    // Seems that we cannot have expecteds of primitives ... this is a problem as samples can be primitive types.
}

