// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
//
// This program and the accompanying materials are made available under the
// terms of the Apache Software License 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0, or the MIT license
// which is available at https://opensource.org/licenses/MIT.
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "iox/unnamed_semaphore.hpp"
#include "test.hpp"
#include "test_posix_semaphore_common.hpp"

namespace
{
using namespace ::testing;
using namespace iox;

class UnnamedSemaphoreTest : public Test
{
  public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    optional<UnnamedSemaphore> sut;
};

TEST_F(UnnamedSemaphoreTest, DefaultInitialValueIsZero)
{
    ::testing::Test::RecordProperty("TEST_ID", "33b6c6b9-ef33-4c62-a03b-f4405cfa2414");
    ASSERT_FALSE(UnnamedSemaphoreBuilder().create(sut).has_error());
    EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut, 0U));
}

TEST_F(UnnamedSemaphoreTest, InitialValueIsSetOnCreation)
{
    ::testing::Test::RecordProperty("TEST_ID", "33e6a780-f115-4477-b78d-34cdfc89a824");
    for (uint32_t initialValue = 313U; initialValue < 10000U; initialValue *= 3U)
    {
        ASSERT_FALSE(UnnamedSemaphoreBuilder().initialValue(initialValue).create(sut).has_error());
        EXPECT_TRUE(setSemaphoreToZeroAndVerifyValue(*sut, initialValue));
    }
}
} // namespace
