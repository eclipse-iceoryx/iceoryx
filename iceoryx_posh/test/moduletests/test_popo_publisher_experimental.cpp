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

#include "iceoryx_posh/experimental/popo/publisher.hpp"
#include "iceoryx_posh/capro/service_description.hpp"

#include "test.hpp"

#include <iostream>

using namespace ::testing;

class ExperimentalPublisherTest : public Test {

public:
    ExperimentalPublisherTest()
    {

    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

};

TEST_F(ExperimentalPublisherTest, BasicUidRetrieval)
{
    struct Position {
        double_t x = 0.0;
        double_t y = 0.0;
        double_t z = 0.0;
    } position;

    iox::popo::Publisher<Position> publisher{};

    auto uid = publisher.uid();
    std::cout << "Publisher UID: " << uid << std::endl;

}

TEST_F(ExperimentalPublisherTest, BasicAllocateWriteAndPublish)
{
    struct Position {
        double_t x = 0.0;
        double_t y = 0.0;
        double_t z = 0.0;
    };

    iox::popo::Publisher<Position> publisher{};
    publisher.allocate()
        .and_then([&](Position* chunk){
            auto position = new (chunk) Position();
            position->x = 42.42;
            position->y = 77.77;
            position->z = 00.15;
            publisher.publish(std::move(chunk));
        });
}

TEST_F(ExperimentalPublisherTest, BasicCopiedPublish)
{
    struct Position {
        double_t x = 0.0;
        double_t y = 0.0;
        double_t z = 0.0;
    } position;

    iox::popo::Publisher<Position> publisher{};
    publisher.publishCopyOf(position);

}

TEST_F(ExperimentalPublisherTest, BasicAllocateThenPublish)
{
    struct Position {
        double_t x = 0.0;
        double_t y = 0.0;
        double_t z = 0.0;
    } position;

    iox::popo::Publisher<Position> publisher{};
    publisher.allocate([&](Position* chunk){
        std::cout << "Lambda called with passed chunk." << std::endl;
    });

}
