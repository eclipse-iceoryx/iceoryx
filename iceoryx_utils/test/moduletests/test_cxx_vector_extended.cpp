// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/cxx/vector.hpp"
#include "test.hpp"


using namespace ::testing;
using namespace iox::cxx;

namespace
{
class vector_extended_test : public Test
{
  public:
    static uint64_t cTor;
    static uint64_t customCTor;
    static uint64_t copyCTor;
    static uint64_t moveCTor;
    static uint64_t moveAssignment;
    static uint64_t copyAssignment;
    static uint64_t dTor;
    static uint64_t classValue;

    class CTorTest
    {
      public:
        CTorTest()
        {
            cTor++;
        }

        CTorTest(const int64_t value)
            : value(value)
        {
            customCTor++;
        }

        CTorTest(const CTorTest& rhs)
        {
            copyCTor++;
            value = rhs.value;
        }

        CTorTest(CTorTest&& rhs)
        {
            moveCTor++;
            value = rhs.value;
        }

        CTorTest& operator=(const CTorTest& rhs)
        {
            copyAssignment++;
            value = rhs.value;
            return *this;
        }

        CTorTest& operator=(CTorTest&& rhs)
        {
            moveAssignment++;
            value = rhs.value;
            return *this;
        }

        ~CTorTest()
        {
            dTor++;
            classValue = value;
        }

        int64_t value = 0;
    };

    void SetUp()
    {
        cTor = 0;
        customCTor = 0;
        copyCTor = 0;
        moveCTor = 0;
        moveAssignment = 0;
        copyAssignment = 0;
        dTor = 0;
        classValue = 0;
    }

    vector<CTorTest, 10> sut;
};


uint64_t vector_extended_test::cTor;
uint64_t vector_extended_test::customCTor;
uint64_t vector_extended_test::copyCTor;
uint64_t vector_extended_test::moveCTor;
uint64_t vector_extended_test::moveAssignment;
uint64_t vector_extended_test::copyAssignment;
uint64_t vector_extended_test::dTor;
uint64_t vector_extended_test::classValue;

TEST_F(vector_extended_test, DestructorOnErase)
{
    for (uint64_t i = 0U; i < 10U; ++i)
    {
        sut.emplace_back(i);
    }
    auto iter = sut.begin(); // element w/ value ==0
    ++iter;                  // element w/ value ==1
    ++iter;                  // element w/ value ==2
    sut.erase(iter);

    EXPECT_THAT(dTor, Eq(1U));
    EXPECT_THAT(classValue, Eq(2U));
}

} // namespace
