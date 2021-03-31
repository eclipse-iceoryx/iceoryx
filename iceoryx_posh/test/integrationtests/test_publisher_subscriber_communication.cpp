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

#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/forward_list.hpp"
#include "iceoryx_utils/cxx/list.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/stack.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/cxx/variant.hpp"
#include "iceoryx_utils/cxx/vector.hpp"
#include "testutils/roudi_gtest.hpp"

#include "test.hpp"

using namespace ::testing;
using ::testing::Return;

using namespace iox;
using namespace iox::popo;
using namespace iox::cxx;

namespace
{
template <typename T>
struct ComplexDataType
{
    int64_t someNumber = 0;
    T complexType;
};

class PublisherSubscriberCommunication_test : public RouDi_GTest
{
  public:
    void SetUp()
    {
        runtime::PoshRuntime::initRuntime("PublisherSubscriberCommunication_test");
    };
    void TearDown(){};

    template <typename T>
    std::unique_ptr<iox::popo::Publisher<T>> createPublisher()
    {
        return std::make_unique<iox::popo::Publisher<T>>(m_serviceDescription);
    }

    template <typename T>
    std::unique_ptr<iox::popo::Subscriber<T>> createSubscriber()
    {
        return std::make_unique<iox::popo::Subscriber<T>>(m_serviceDescription);
    }


    capro::ServiceDescription m_serviceDescription{
        "PublisherSubscriberCommunication", "IntegrationTest", "AllHailHypnotoad"};
};
} // namespace

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_forward_list)
{
    using Type_t = ComplexDataType<forward_list<string<5>, 5>>;
    auto publisher = createPublisher<Type_t>();
    this->InterOpWait();
    auto subscriber = createSubscriber<Type_t>();
    this->InterOpWait();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 123;
                         sample->complexType.push_front("world");
                         sample->complexType.push_front("hello");
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(123));
                         ASSERT_THAT(sample->complexType.size(), Eq(2U));
                         auto begin = sample->complexType.begin();
                         EXPECT_THAT(*begin, Eq(cxx::string<5>("hello")));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(cxx::string<5>("world")));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_list)
{
    using Type_t = ComplexDataType<list<int64_t, 5>>;
    auto publisher = createPublisher<Type_t>();
    this->InterOpWait();
    auto subscriber = createSubscriber<Type_t>();
    this->InterOpWait();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 4123;
                         sample->complexType.push_front(77);
                         sample->complexType.push_front(66);
                         sample->complexType.push_front(55);
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(4123));
                         ASSERT_THAT(sample->complexType.size(), Eq(3U));
                         auto begin = sample->complexType.begin();
                         EXPECT_THAT(*begin, Eq(55));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(66));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(77));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_optional)
{
    using Type_t = ComplexDataType<list<optional<int32_t>, 5>>;
    auto publisher = createPublisher<Type_t>();
    this->InterOpWait();
    auto subscriber = createSubscriber<Type_t>();
    this->InterOpWait();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 41231;
                         sample->complexType.push_front(177);
                         sample->complexType.push_front(nullopt);
                         sample->complexType.push_front(155);
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(41231));
                         ASSERT_THAT(sample->complexType.size(), Eq(3U));
                         auto begin = sample->complexType.begin();
                         EXPECT_THAT(*begin, Eq(cxx::optional<int32_t>(155)));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(nullopt));
                         ++begin;
                         EXPECT_THAT(*begin, Eq(cxx::optional<int32_t>(177)));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_stack)
{
    using Type_t = ComplexDataType<stack<int64_t, 10>>;
    auto publisher = createPublisher<Type_t>();
    this->InterOpWait();
    auto subscriber = createSubscriber<Type_t>();
    this->InterOpWait();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 41231;
                         for (uint64_t i = 0U; i < 10U; ++i)
                         {
                             sample->complexType.push(i + 123);
                         }
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(41231));
                         ASSERT_THAT(sample->complexType.size(), Eq(10U));
                         auto stackCopy = sample->complexType;
                         for (uint64_t i = 0U; i < 10U; ++i)
                         {
                             auto result = stackCopy.pop();
                             ASSERT_THAT(result.has_value(), Eq(true));
                             EXPECT_THAT(*result, Eq(123 + 9 - i));
                         }
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_string)
{
    using Type_t = ComplexDataType<string<128>>;
    auto publisher = createPublisher<Type_t>();
    this->InterOpWait();
    auto subscriber = createSubscriber<Type_t>();
    this->InterOpWait();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 123;
                         sample->complexType = "You're my Heart, You're my Seal!";
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(123));
                         EXPECT_THAT(sample->complexType, Eq(string<128>("You're my Heart, You're my Seal!")));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_vector)
{
    using Type_t = ComplexDataType<vector<string<128>, 20>>;
    auto publisher = createPublisher<Type_t>();
    this->InterOpWait();
    auto subscriber = createSubscriber<Type_t>();
    this->InterOpWait();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 123;
                         sample->complexType.emplace_back("Don't stop the hypnotoad");
                         sample->complexType.emplace_back("Be like hypnotoad");
                         sample->complexType.emplace_back("Piep, piep little satellite");
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(123));
                         ASSERT_THAT(sample->complexType.size(), Eq(3));
                         EXPECT_THAT(sample->complexType[0], Eq(string<128>("Don't stop the hypnotoad")));
                         EXPECT_THAT(sample->complexType[1], Eq(string<128>("Be like hypnotoad")));
                         EXPECT_THAT(sample->complexType[2], Eq(string<128>("Piep, piep little satellite")));
                     })
                     .has_error());
}

TEST_F(PublisherSubscriberCommunication_test, SendingComplexDataType_variant)
{
    using Type_t = ComplexDataType<vector<variant<string<128>, int>, 20>>;
    auto publisher = createPublisher<Type_t>();
    this->InterOpWait();
    auto subscriber = createSubscriber<Type_t>();
    this->InterOpWait();

    ASSERT_FALSE(publisher->loan()
                     .and_then([](auto& sample) {
                         sample->someNumber = 123;
                         sample->complexType.emplace_back(in_place_index<0>(), "Be aware! Bob is a vampire!");
                         sample->complexType.emplace_back(in_place_index<1>(), 1337);
                         sample->complexType.emplace_back(in_place_index<0>(), "Bob is an acronym for Bob Only Bob");
                         sample.publish();
                     })
                     .has_error());

    EXPECT_FALSE(subscriber->take()
                     .and_then([](auto& sample) {
                         EXPECT_THAT(sample->someNumber, Eq(123));
                         ASSERT_THAT(sample->complexType.size(), Eq(3));
                         ASSERT_THAT(sample->complexType[0].index(), Eq(0));
                         EXPECT_THAT(*sample->complexType[0].template get_at_index<0>(),
                                     Eq(string<128>("Be aware! Bob is a vampire!")));
                         ASSERT_THAT(sample->complexType[1].index(), Eq(1));
                         EXPECT_THAT(*sample->complexType[1].template get_at_index<1>(), Eq(1337));
                         ASSERT_THAT(sample->complexType[2].index(), Eq(0));
                         EXPECT_THAT(*sample->complexType[2].template get_at_index<0>(),
                                     Eq(string<128>("Bob is an acronym for Bob Only Bob")));
                     })
                     .has_error());
}

