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

#include "iceoryx_posh/popo/modern_api/base_publisher.hpp"
#include "iceoryx_posh/popo/modern_api/sample.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"
#include "test.hpp"

using namespace ::testing;
using ::testing::_;

struct DummyData
{
    DummyData() = default;
    int val = 42;
};

template <typename T>
class MockPublisherInterface : public iox::popo::PublisherInterface<T>
{
  public:
    void publish(iox::popo::Sample<T>&& sample) noexcept
    {
        return publishMock(std::move(sample));
    }
    MOCK_METHOD1_T(publishMock, void(iox::popo::Sample<T>&&));
};

class SampleTest : public Test
{
  public:
    SampleTest()
    {
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

  protected:
};

TEST_F(SampleTest, PublishesSampleViaPublisherInterface)
{
    // ===== Setup ===== //
    iox::cxx::unique_ptr<DummyData> testSamplePtr{new DummyData(), [](DummyData* ptr) { delete ptr; }};
    MockPublisherInterface<DummyData> mockPublisherInterface{};

    auto sut = iox::popo::Sample<DummyData>(std::move(testSamplePtr), mockPublisherInterface);

    EXPECT_CALL(mockPublisherInterface, publishMock).Times(1);

    // ===== Test ===== //
    sut.publish();

    // ===== Verify ===== //
    // ===== Cleanup ===== //
}
