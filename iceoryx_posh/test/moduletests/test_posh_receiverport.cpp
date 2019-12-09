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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "test.hpp"

using namespace ::testing;
using namespace iox::popo;
using namespace iox::capro;
using CString100 = iox::cxx::CString100;
using CaproMessage = iox::capro::CaproMessage;

class ReceiverPort_test : public Test
{
  protected:
    ReceiverPort_test()
        : m_memoryAllocator(m_memory, 1024 * 1024)
    {
        ActivateSender(m_sender);

        mempoolconf.addMemPool({32, 20});
        m_memPoolHandler.configureMemoryManager(mempoolconf, &m_memoryAllocator, &m_memoryAllocator);
    }

    ~ReceiverPort_test()
    {
        for (auto port : m_ports)
        {
            delete port;
        }
        for (auto member : m_portData)
        {
            delete member;
        }
    }

    void SetUp()
    {
    }

    void TearDown()
    {
    }

    void SubscribeReceiverToSender(iox::ReceiverPortType* f_receiver, iox::SenderPortType* f_sender)
    {
        CaproMessage expect_sub_msg = {iox::capro::CaproMessageType::SUB, m_service};

        /// send subscription request to roudiPort
        f_receiver->subscribe(true, 10);

        auto returnedCaproMessage = f_receiver->getCaProMessage();
        EXPECT_THAT(returnedCaproMessage.has_value(), Eq(true));
        if (returnedCaproMessage.has_value())
        {
            auto msg = returnedCaproMessage.value();
            EXPECT_THAT(msg.m_type, Eq(expect_sub_msg.m_type));
            EXPECT_THAT(msg.m_serviceDescription, Eq(expect_sub_msg.m_serviceDescription));

            auto senderResponse = f_sender->dispatchCaProMessage(msg);
            EXPECT_THAT(senderResponse.has_value(), Eq(true));
            if (senderResponse.has_value())
            {
                CaproMessage expect_ack_msg = {iox::capro::CaproMessageType::ACK, m_service};
                msg = senderResponse.value();
                EXPECT_THAT(msg.m_type, Eq(expect_ack_msg.m_type));
                EXPECT_THAT(msg.m_serviceDescription, Eq(expect_ack_msg.m_serviceDescription));
                EXPECT_THAT(f_receiver->isSubscribed(), Eq(true));
            }
        }
    }

    iox::ReceiverPortType* CreateReceiver(const ServiceDescription& f_service)
    {
        iox::ReceiverPortType::MemberType_t* data = new iox::ReceiverPortType::MemberType_t(f_service, "", iox::Interfaces::INTERNAL, nullptr);
        m_portData.emplace_back(data);
        iox::ReceiverPortType* l_receiver = new iox::ReceiverPortType(data);
        m_ports.emplace_back(l_receiver);
        return l_receiver;
    }

    iox::SenderPortType* CreateSender(const ServiceDescription& f_service)
    {
        iox::SenderPortType::MemberType_t* data =
            new iox::SenderPortType::MemberType_t(f_service, &m_memPoolHandler, "", iox::Interfaces::INTERNAL, nullptr);
        m_portData.emplace_back(data);
        iox::SenderPortType* l_sender = new iox::SenderPortType(data);
        m_ports.emplace_back(l_sender);

        return l_sender;
    }

    void ActivateSender(iox::SenderPortType* const f_sender)
    {
        f_sender->activate();
        CaproMessage expect_offer_msg = {iox::capro::CaproMessageType::OFFER, m_service};

        auto returnedCaproMessage = f_sender->getCaProMessage();
        EXPECT_THAT(returnedCaproMessage.has_value(), Eq(true));
        if (returnedCaproMessage.has_value())
        {
            auto msg = returnedCaproMessage.value();
            EXPECT_THAT(msg.m_type, Eq(expect_offer_msg.m_type));
        }
    }

    char m_memory[1024 * 1024];
    std::vector<BasePort*> m_ports;
    std::vector<BasePortData*> m_portData;
    iox::posix::Allocator m_memoryAllocator;
    iox::mepoo::MemoryManager m_memPoolHandler;
    ServiceDescription m_service{1, 1, 1};
    iox::SenderPortType* m_sender = CreateSender(m_service);
    iox::ReceiverPortType* m_receiver = CreateReceiver(m_service);
    iox::mepoo::MePooConfig mempoolconf;
};

TEST_F(ReceiverPort_test, newdata)
{
    SubscribeReceiverToSender(m_receiver, m_sender);

    EXPECT_THAT(m_receiver->newData(), Eq(false));
    int l_data = 100;
    auto l_delivery = m_sender->reserveChunk(sizeof(l_data));
    l_delivery->m_info.m_payloadSize = sizeof(l_data);
    m_sender->deliverChunk(l_delivery);
    EXPECT_THAT(m_receiver->newData(), Eq(true));
}

TEST_F(ReceiverPort_test, releaseChunk)
{
    SubscribeReceiverToSender(m_receiver, m_sender);

    int l_data = 100;
    auto l_delivery = m_sender->reserveChunk(sizeof(l_data));
    l_delivery->m_info.m_payloadSize = sizeof(l_data);
    m_sender->deliverChunk(l_delivery);
    EXPECT_THAT(m_receiver->newData(), Eq(true));

    iox::mepoo::SharedChunk receivedSample;
    const iox::mepoo::ChunkHeader* chunkHeader;
    ASSERT_THAT(m_receiver->getChunk(chunkHeader), true);

    EXPECT_THAT(chunkHeader->m_info.m_payloadSize, Eq(sizeof(l_data)));

    EXPECT_THAT(m_receiver->releaseChunk(chunkHeader), true);
    EXPECT_THAT(m_receiver->newData(), Eq(false));
}

// test the state machine logic (unit test does not account for concurrency)
// here the common use cases are tested
///@todo: do we cover all relevant cases?

// standard subsribe/unsubscribe case
TEST_F(ReceiverPort_test, subscription)
{
    using State = iox::SubscribeState;
    auto state = m_receiver->getSubscribeState();

    EXPECT_EQ(m_receiver->getSubscribeState(), State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    m_receiver->subscribe(true, 10);

    EXPECT_EQ(state, State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    auto response = m_receiver->getCaProMessage();

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBE_REQUESTED);
    EXPECT_FALSE(m_receiver->isSubscribed());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->m_type, CaproMessageType::SUB);

    auto service = m_receiver->getCaProServiceDescription();
    CaproMessage message(CaproMessageType::ACK, service);
    response = m_receiver->dispatchCaProMessage(message);

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBED);
    EXPECT_TRUE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());

    // subscribed, now unsubscribe (all in one test to save execution time)

    m_receiver->unsubscribe();

    response = m_receiver->getCaProMessage();

    EXPECT_EQ(m_receiver->getSubscribeState(), State::UNSUBSCRIBE_REQUESTED);
    EXPECT_TRUE(m_receiver->isSubscribed());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->m_type, CaproMessageType::UNSUB);

    response = m_receiver->dispatchCaProMessage(message);

    EXPECT_EQ(m_receiver->getSubscribeState(), State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());
}

// test subscribing multiple times
TEST_F(ReceiverPort_test, multiSubscription)
{
    using State = iox::SubscribeState;

    EXPECT_EQ(m_receiver->getSubscribeState(), State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    m_receiver->subscribe(true, 10);

    EXPECT_EQ(m_receiver->getSubscribeState(), State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    // subscription pending, try subscribing again
    m_receiver->subscribe(true, 10);

    EXPECT_EQ(m_receiver->getSubscribeState(), State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    auto response = m_receiver->getCaProMessage();

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBE_REQUESTED);
    EXPECT_FALSE(m_receiver->isSubscribed());
    EXPECT_TRUE(response.has_value());

    auto service = m_receiver->getCaProServiceDescription();
    CaproMessage message(CaproMessageType::ACK, service);
    response = m_receiver->dispatchCaProMessage(message);

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBED);
    EXPECT_TRUE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());

    // subscribed, but subscribe again

    m_receiver->subscribe(true, 10);

    response = m_receiver->getCaProMessage();

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBED);
    EXPECT_TRUE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());

    // from here on we already tested the unsubscribe transitions in the subscription test case
}


// delay subsription which leads to wait for offer and then subscribe later
TEST_F(ReceiverPort_test, delayedSubscription)
{
    using State = iox::SubscribeState;
    auto state = m_receiver->getSubscribeState();

    EXPECT_EQ(m_receiver->getSubscribeState(), State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    m_receiver->subscribe(true, 10);

    EXPECT_EQ(state, State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    auto response = m_receiver->getCaProMessage();

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBE_REQUESTED);
    EXPECT_FALSE(m_receiver->isSubscribed());
    EXPECT_TRUE(response.has_value());

    auto service = m_receiver->getCaProServiceDescription();
    response = m_receiver->dispatchCaProMessage(CaproMessage(CaproMessageType::NACK, service));

    EXPECT_EQ(m_receiver->getSubscribeState(), State::WAIT_FOR_OFFER);
    EXPECT_FALSE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());

    response = m_receiver->dispatchCaProMessage(CaproMessage(CaproMessageType::OFFER, service));

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBE_REQUESTED);
    EXPECT_FALSE(m_receiver->isSubscribed());
    EXPECT_TRUE(response.has_value());

    response = m_receiver->dispatchCaProMessage(CaproMessage(CaproMessageType::ACK, service));

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBED);
    EXPECT_TRUE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());
}


// subscribe and then stop offering, leading to unsubscribed receiver port
// re-offer leads to subscribed port again
TEST_F(ReceiverPort_test, stopOffer)
{
    using State = iox::SubscribeState;

    EXPECT_EQ(m_receiver->getSubscribeState(), State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    m_receiver->subscribe(true, 10);

    EXPECT_EQ(m_receiver->getSubscribeState(), State::NOT_SUBSCRIBED);
    EXPECT_FALSE(m_receiver->isSubscribed());

    auto response = m_receiver->getCaProMessage();

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBE_REQUESTED);
    EXPECT_FALSE(m_receiver->isSubscribed());
    EXPECT_TRUE(response.has_value());

    auto service = m_receiver->getCaProServiceDescription();
    response = m_receiver->dispatchCaProMessage(CaproMessage(CaproMessageType::ACK, service));

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBED);
    EXPECT_TRUE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());

    response = m_receiver->dispatchCaProMessage(CaproMessage(CaproMessageType::STOP_OFFER, service));

    EXPECT_EQ(m_receiver->getSubscribeState(), State::WAIT_FOR_OFFER);
    EXPECT_FALSE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());

    // re-offer and re-subscribe

    response = m_receiver->dispatchCaProMessage(CaproMessage(CaproMessageType::OFFER, service));

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBE_REQUESTED);
    EXPECT_FALSE(m_receiver->isSubscribed());
    EXPECT_TRUE(response.has_value());

    response = m_receiver->dispatchCaProMessage(CaproMessage(CaproMessageType::ACK, service));

    EXPECT_EQ(m_receiver->getSubscribeState(), State::SUBSCRIBED);
    EXPECT_TRUE(m_receiver->isSubscribed());
    EXPECT_FALSE(response.has_value());
}
