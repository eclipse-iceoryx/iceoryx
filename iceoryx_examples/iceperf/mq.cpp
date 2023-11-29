// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "mq.hpp"
#include "iceoryx_platform/fcntl.hpp"
#include "iceoryx_platform/platform_correction.hpp"
#include "iox/posix_call.hpp"
#include "iox/std_string_support.hpp"

#include <chrono>
#include <thread>

MQ::MQ(const std::string& publisherName, const std::string& subscriberName) noexcept
    : m_publisherMqName(PREFIX + publisherName)
    , m_subscriberMqName(PREFIX + subscriberName)
{
    initMqAttributes();
}

void MQ::cleanupOutdatedResources(const std::string& publisherName, const std::string& subscriberName) noexcept
{
    auto publisherMqName = PREFIX + publisherName;
    IOX_POSIX_CALL(mq_unlink)
    (publisherMqName.c_str()).failureReturnValue(ERROR_CODE).ignoreErrnos(ENOENT).evaluate().or_else([&](auto& r) {
        std::cout << "mq_unlink error for " << publisherMqName << ", " << r.getHumanReadableErrnum() << std::endl;
        exit(1);
    });

    auto subscriberMqName = PREFIX + subscriberName;
    IOX_POSIX_CALL(mq_unlink)
    (subscriberMqName.c_str()).failureReturnValue(ERROR_CODE).ignoreErrnos(ENOENT).evaluate().or_else([&](auto& r) {
        std::cout << "mq_unlink error for " << subscriberMqName << ", " << r.getHumanReadableErrnum() << std::endl;
        exit(1);
    });
}

void MQ::initLeader() noexcept
{
    open(m_subscriberMqName, iox::PosixIpcChannelSide::SERVER);

    std::cout << "waiting for follower" << std::endl;

    receivePerfTopic();

    open(m_publisherMqName, iox::PosixIpcChannelSide::CLIENT);
}

void MQ::initFollower() noexcept
{
    open(m_subscriberMqName, iox::PosixIpcChannelSide::SERVER);

    std::cout << "registering with the leader" << std::endl;

    open(m_publisherMqName, iox::PosixIpcChannelSide::CLIENT);

    sendPerfTopic(sizeof(PerfTopic), RunFlag::RUN);
}

void MQ::initMqAttributes() noexcept
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
    IOX_POSIX_CALL(mq_close)
    (m_mqDescriptorSubscriber).failureReturnValue(ERROR_CODE).evaluate().or_else([&](auto& r) {
        std::cout << "mq_close error for " << m_subscriberMqName << ", " << r.getHumanReadableErrnum() << std::endl;
        exit(1);
    });

    IOX_POSIX_CALL(mq_unlink)
    (m_subscriberMqName.c_str()).failureReturnValue(ERROR_CODE).ignoreErrnos(ENOENT).evaluate().or_else([&](auto& r) {
        std::cout << "mq_unlink error for " << m_subscriberMqName << ", " << r.getHumanReadableErrnum() << std::endl;
        exit(1);
    });

    IOX_POSIX_CALL(mq_close)
    (m_mqDescriptorPublisher).failureReturnValue(ERROR_CODE).evaluate().or_else([&](auto& r) {
        std::cout << "mq_close error for " << m_publisherMqName << ", " << r.getHumanReadableErrnum() << std::endl;
        exit(1);
    });
}

void MQ::sendPerfTopic(const uint32_t payloadSizeInBytes, const RunFlag runFlag) noexcept
{
    char* buffer = new char[payloadSizeInBytes];
    auto sample = reinterpret_cast<PerfTopic*>(&buffer[0]);

    // Specify the payload size for the measurement
    sample->payloadSize = payloadSizeInBytes;
    sample->runFlag = runFlag;
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

void MQ::open(const std::string& name, const iox::PosixIpcChannelSide channelSide) noexcept
{
    int32_t openFlags = O_RDWR | O_NONBLOCK;
    if (channelSide == iox::PosixIpcChannelSide::SERVER)
    {
        openFlags |= O_CREAT;
    }

    constexpr bool TRY_TO_OPEN{true};
    while (TRY_TO_OPEN)
    {
        // the mask will be applied to the permissions, therefore we need to set it to 0
        mode_t umaskSaved = umask(0);

        auto mqCall = IOX_POSIX_CALL(iox_mq_open4)(name.c_str(), openFlags, m_filemode, &m_attributes)
                          .failureReturnValue(INVALID_DESCRIPTOR)
                          .ignoreErrnos(ENOENT)
                          .evaluate()
                          .or_else([&](auto& r) {
                              std::cout << "mq_open error for " << name << ", " << r.getHumanReadableErrnum()
                                        << std::endl;
                              exit(1);
                          });
        umask(umaskSaved);

        if (mqCall->errnum == ENOENT)
        {
            constexpr std::chrono::milliseconds RETRY_INTERVAL{10};
            std::this_thread::sleep_for(RETRY_INTERVAL);
            continue;
        }

        if (channelSide == iox::PosixIpcChannelSide::SERVER)
        {
            m_mqDescriptorSubscriber = mqCall->value;
        }
        else
        {
            m_mqDescriptorPublisher = mqCall->value;
        }

        break;
    }
}

void MQ::send(const char* buffer, uint32_t length) noexcept
{
    while (IOX_POSIX_CALL(mq_send)(m_mqDescriptorPublisher, buffer, length, 1U)
               .failureReturnValue(ERROR_CODE)
               .ignoreErrnos(EAGAIN)
               .evaluate()
               .or_else([&](auto& r) {
                   std::cout << std::endl
                             << "send error for " << m_publisherMqName << ", " << r.getHumanReadableErrnum()
                             << std::endl;
                   exit(1);
               })
               ->errnum
           == EAGAIN)
    {
    }
}

void MQ::receive(char* buffer) noexcept
{
    while (IOX_POSIX_CALL(mq_receive)(m_mqDescriptorSubscriber, buffer, MAX_MESSAGE_SIZE, nullptr)
               .failureReturnValue(ERROR_CODE)
               .ignoreErrnos(EAGAIN)
               .evaluate()
               .or_else([&](auto& r) {
                   std::cout << "receive error for " << m_subscriberMqName << ", " << r.getHumanReadableErrnum()
                             << std::endl;
                   exit(1);
               })
               ->errnum
           == EAGAIN)
    {
    }
}
