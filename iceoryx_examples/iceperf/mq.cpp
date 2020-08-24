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

#include "mq.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/internal/posix_wrapper/message_queue.hpp"
#include "iceoryx_utils/platform/fcntl.hpp"
#include "iceoryx_utils/platform/platform_correction.hpp"

#include <chrono>

MQ::MQ(const std::string& publisherName, const std::string& subscriberName) noexcept
    : m_publisherName(publisherName)
    , m_subscriberName(subscriberName)
{
}

void MQ::initLeader() noexcept
{
    init();

    open(m_subscriberName, iox::posix::IpcChannelSide::SERVER);

    std::cout << "waiting for follower" << std::endl;

    receivePerfTopic();

    open(m_publisherName, iox::posix::IpcChannelSide::CLIENT);
}

void MQ::initFollower() noexcept
{
    init();

    open(m_subscriberName, iox::posix::IpcChannelSide::SERVER);

    std::cout << "registering with the leader, if no leader this will crash with a message queue error now"
              << std::endl;

    open(m_publisherName, iox::posix::IpcChannelSide::CLIENT);

    sendPerfTopic(sizeof(PerfTopic), true);
}

void MQ::init() noexcept
{
    // fields have a different order in QNX,
    // so we need to initialize by name
    m_attributes.mq_flags = 0;
    m_attributes.mq_maxmsg = MAX_MESSAGES;
    m_attributes.mq_msgsize = MAX_MESSAGE_SIZE;
    m_attributes.mq_curmsgs = 0;
#ifdef __QNX__
    m_attributes.mq_recvwait = 0;
    m_attributes.mq_sendwait = 0;
#endif
}

void MQ::shutdown() noexcept
{
    auto mqCallSubClose = iox::cxx::makeSmartC(
        mq_close, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, m_mqDescriptorSubscriber);

    if (mqCallSubClose.hasErrors())
    {
        std::cout << "mq_close error" << std::endl;
        exit(1);
    }

    auto mqCallSubUnlink = iox::cxx::makeSmartC(
        mq_unlink, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {ENOENT}, m_subscriberName.c_str());

    if (mqCallSubUnlink.hasErrors())
    {
        std::cout << "mq_unlink error" << std::endl;
        exit(1);
    }

    auto mqCallPubClose = iox::cxx::makeSmartC(
        mq_close, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {}, m_mqDescriptorPublisher);

    if (mqCallPubClose.hasErrors())
    {
        std::cout << "mq_close error" << std::endl;
        exit(1);
    }
}

void MQ::sendPerfTopic(uint32_t payloadSizeInBytes, bool runFlag) noexcept
{
    char* buffer = new char[payloadSizeInBytes];
    auto sample = reinterpret_cast<PerfTopic*>(&buffer[0]);

    // Specify the payload size for the measurement
    sample->payloadSize = payloadSizeInBytes;
    sample->run = runFlag;
    if (payloadSizeInBytes <= MAX_MESSAGE_SIZE)
    {
        sample->subPackets = 1;
        send(&buffer[0], payloadSizeInBytes);
    }
    else
    {
        sample->subPackets = payloadSizeInBytes / MAX_MESSAGE_SIZE;
        for (uint32_t i = 0U; i < sample->subPackets; ++i)
        {
            send(&buffer[0], MAX_MESSAGE_SIZE);
        }
    }
    delete[] buffer;
}

PerfTopic MQ::receivePerfTopic() noexcept
{
    receive(&m_message[0]);

    auto receivedSample = reinterpret_cast<const PerfTopic*>(&m_message[0]);

    if (receivedSample->subPackets > 1)
    {
        for (uint32_t i = 0U; i < receivedSample->subPackets - 1; ++i)
        {
            receive(&m_message[0]);
        }
    }

    return *receivedSample;
}

void MQ::open(const std::string& name, const iox::posix::IpcChannelSide channelSide) noexcept
{
    int32_t openFlags = O_RDWR;
    if (channelSide == iox::posix::IpcChannelSide::SERVER)
    {
        auto mqCall = iox::cxx::makeSmartC(
            mq_unlink, iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {ERROR_CODE}, {ENOENT}, name.c_str());
        if (mqCall.hasErrors())
        {
            std::cout << "mq_unlink error" << std::endl;
            exit(1);
        }

        openFlags |= O_CREAT;
    }

    // the mask will be applied to the permissions, therefore we need to set it to 0
    mode_t umaskSaved = umask(0);

    auto mqCall = iox::cxx::makeSmartC(mq_open,
                                       iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                       {ERROR_CODE},
                                       {ENOENT},
                                       name.c_str(),
                                       openFlags,
                                       m_filemode,
                                       &m_attributes);

    umask(umaskSaved);

    if (mqCall.hasErrors())
    {
        std::cout << "mq_open error " << name << std::endl;
        exit(1);
    }

    if (channelSide == iox::posix::IpcChannelSide::SERVER)
    {
        m_mqDescriptorSubscriber = mqCall.getReturnValue();
    }
    else
    {
        m_mqDescriptorPublisher = mqCall.getReturnValue();
    }
}

void MQ::send(const char* buffer, uint32_t length) noexcept
{
    auto mqCall = iox::cxx::makeSmartC(mq_send,
                                       iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                       {ERROR_CODE},
                                       {},
                                       m_mqDescriptorPublisher,
                                       buffer,
                                       length,
                                       1);

    if (mqCall.hasErrors())
    {
        std::cout << std::endl << "send error" << std::endl;
        exit(1);
    }
}

void MQ::receive(char* buffer) noexcept
{
    char message[MAX_MESSAGE_SIZE];
    auto mqCall = iox::cxx::makeSmartC(mq_receive,
                                       iox::cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                       {static_cast<ssize_t>(ERROR_CODE)},
                                       {},
                                       m_mqDescriptorSubscriber,
                                       buffer,
                                       MAX_MESSAGE_SIZE,
                                       nullptr);

    if (mqCall.hasErrors())
    {
        std::cout << "receive error" << std::endl;
        exit(1);
    }
}
