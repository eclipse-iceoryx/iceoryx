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

#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/cxx/convert.hpp"
#include "iceoryx_utils/cxx/smart_c.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"
#include "iceoryx_utils/internal/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_utils/internal/posix_wrapper/timespec.hpp"

#include <thread>

namespace iox
{
namespace runtime
{
std::string mqMessageTypeToString(const MqMessageType msg) noexcept
{
    return std::to_string(static_cast<uint16_t>(msg));
}

MqMessageType stringToMqMessageType(const char* str) noexcept
{
    auto msg = std::strtoll(str, nullptr, 10);
    if (static_cast<int32_t>(MqMessageType::BEGIN) >= msg || static_cast<int32_t>(MqMessageType::END) <= msg)
    {
        return MqMessageType::NOTYPE;
    }
    else
    {
        return static_cast<MqMessageType>(msg);
    }
}

MqBase::MqBase(const std::string& InterfaceName, const long maxMessages, const long messageSize) noexcept
    : m_interfaceName(InterfaceName)
{
    auto maxMessageSize = messageSize;
    if (messageSize > MAX_MESSAGE_LENGTH)
    {
        LogWarn() << "Message size too large, reducing from " << messageSize << " to " << MAX_MESSAGE_LENGTH;
        maxMessageSize = MAX_MESSAGE_LENGTH;
    }
    // initialize m_attr fields by name (in QNX they have a different order)
    m_attr.mq_flags = MQ_FLAGS;
    m_attr.mq_maxmsg = maxMessages;
    m_attr.mq_msgsize = maxMessageSize;
    m_attr.mq_curmsgs = MQ_CUR_MSGS;
#ifdef __QNX__
    m_attr.mq_recvwait = 0;
    m_attr.mq_sendwait = 0;
#endif
}

bool MqBase::receive(MqMessage& answer) const noexcept
{
    char buffer[MAX_MESSAGE_LENGTH];
    if (-1 == mq_receive(m_roudiMq, buffer, static_cast<size_t>(m_attr.mq_msgsize), NULL))
    {
        LogError() << "Calling mq_receive() failed : " << strerror(errno);
        return false;
    }
    else
    {
        return MqBase::setMessageFromString(buffer, answer);
    }
}

bool MqBase::timedReceive(const uint32_t timeout_ms, MqMessage& answer) const noexcept
{
    timespec timeout;
    // get current system time
    if (-1 == (clock_gettime(CLOCK_REALTIME, &timeout)))
    {
        LogError() << "Calling clock_gettime() failed : " << strerror(errno);
        return false;
    }
    timeout = posix::addTimeMs(timeout, timeout_ms);

    char buffer[MAX_MESSAGE_LENGTH];
    if (-1 == mq_timedreceive(m_roudiMq, buffer, static_cast<size_t>(m_attr.mq_msgsize), NULL, &timeout))
    {
        if (errno == ETIMEDOUT || errno == EINTR) // if errno have ETIMEDOUT or EINTR the message queue
                                                  // has only timed out, no message needed
        {
        }
        else // if another errorcode happens, print message
        {
            LogError() << "Calling mq_timedReceive() failed : " << strerror(errno);
        }
        return false;
    }
    else
    {
        return MqBase::setMessageFromString(buffer, answer);
    }
}

bool MqBase::setMessageFromString(const char* buffer, MqMessage& answer) noexcept
{
    answer.setMessage(buffer);
    if (!answer.isValid())
    {
        LogError() << "The received message " << answer.getMessage() << " is not valid";
        return false;
    }
    return true;
}

bool MqBase::send(const MqMessage& msg) const noexcept
{
    if (!msg.isValid())
    {
        LogError() << "Trying to send the message " << msg.getMessage() << "with mq_send() which "
                   << "does not follow the specified syntax.";
        return false;
    }

    const size_t messageSize = static_cast<size_t>(msg.getMessage().size()) + NULL_TERMINATOR_SIZE;
    if (messageSize > static_cast<size_t>(m_attr.mq_msgsize))
    {
        LogError() << "msg size of " << messageSize << "bigger than " << m_attr.mq_msgsize
                   << ", maybe change MQ_MSG_SIZE";
        return false;
    }

    unsigned int prio = 1;
    if (-1 == mq_send(m_roudiMq, msg.getMessage().c_str(), messageSize, prio))
    {
        LogError() << "Calling mq_send() failed : " << strerror(errno);
        return false;
    }
    else
    {
        return true;
    }
}

bool MqBase::timedSend(const MqMessage& msg, units::Duration timeout) const noexcept
{
    if (!msg.isValid())
    {
        LogError() << "Trying to send the message " << msg.getMessage() << " with mq_timedsend() which "
                   << "does not follow the specified syntax.";
        return false;
    }

    const size_t messageSize = msg.getMessage().size() + NULL_TERMINATOR_SIZE;
    if (messageSize > static_cast<size_t>(m_attr.mq_msgsize))
    {
        LogError() << "msg size of " << messageSize << "bigger than " << m_attr.mq_msgsize
                   << ", maybe change MQ_MSG_SIZE";
        return false;
    }

    timespec l_timeout = timeout.timespec(units::TimeSpecReference::Epoch);
    unsigned int l_prio = 1;
    auto sendRetVal = cxx::makeSmartC(mq_timedsend,
                                      cxx::ReturnMode::PRE_DEFINED_ERROR_CODE,
                                      {-1},
                                      {ETIMEDOUT, EINTR},
                                      m_roudiMq,
                                      msg.getMessage().c_str(),
                                      messageSize,
                                      l_prio,
                                      &l_timeout);

    if (sendRetVal.hasErrors())
    {
        LogError() << "Calling mq_timedsend() failed : " << strerror(errno);
        return false;
    }
    else if (errno == ETIMEDOUT || errno == EINTR) // send timed out
    {
        return false;
    }

    return true;
}

const std::string& MqBase::getInterfaceName() const noexcept
{
    return m_interfaceName;
}

bool MqBase::isInitialized() const noexcept
{
    return m_isInitialized;
}

bool MqBase::openMessageQueue(const std::string& name, const int oflag) noexcept
{
    m_oflag = oflag;
    m_roudiMq = mq_open(name.c_str(), m_oflag, m_perms, &m_attr);
    if (-1 == m_roudiMq)
    {
        // error if create fails, open only is also used for checking....
        if (m_oflag & O_CREAT)
        {
            LogWarn() << name << " " << oflag << " " << m_perms;
            LogError() << "Calling mq_open() failed : " << strerror(errno);
        }
        return false;
    }
    return true;
}

bool MqBase::closeMessageQueue() const noexcept
{
    // m_interfaceName is only empty when the object was moved
    if (-1 == mq_close(m_roudiMq))
    {
        LogError() << "Calling mq_close() failed : " << strerror(errno);
        return false;
    }
    return true;
}

bool MqBase::reopen() noexcept
{
    if (m_isInitialized)
    {
        closeMessageQueue();
    }
    m_isInitialized = openMessageQueue(m_interfaceName, m_oflag);
    return m_isInitialized;
}

bool MqBase::mqMapsToFile() noexcept
{
    if (!m_isInitialized)
    {
        return false;
    }
    struct stat sb;
    if (cxx::makeSmartC(fstat, cxx::ReturnMode::PRE_DEFINED_ERROR_CODE, {-1}, {}, m_roudiMq, &sb).hasErrors())
    {
        errorHandler(Error::kMQ_INTERFACE__CHECK_MQ_MAPS_TO_FILE);
    }
    return sb.st_nlink > 0;
}

bool MqBase::unlinkMessageQueue() const noexcept
{
    if (-1 == mq_unlink(m_interfaceName.c_str()))
    {
        LogError() << "Calling mq_unlink() failed : " << strerror(errno);
        return false;
    }
    return true;
}

bool MqBase::hasClosableMessageQueue() const noexcept
{
    /// m_interfaceName is empty when the MqBase object was moved
    return !m_interfaceName.empty();
}

void MqBase::cleanupOutdatedMessageQueue(const std::string& name) noexcept
{
    MqBase mqueue(name, 1, 1);
    if (mqueue.openMessageQueue(name, O_RDWR))
    {
        LogWarn() << "MQ still there, doing an unlink of " << name;
        mqueue.unlinkMessageQueue();
    }
}

MqInterfaceUser::MqInterfaceUser(const std::string& name, const long maxMessages, const long messageSize) noexcept
    : MqBase(name, maxMessages, messageSize)
{
    m_isInitialized = openMessageQueue(name, O_RDWR);
}

MqInterfaceUser::MqInterfaceUser(MqInterfaceUser&& rhs) noexcept
    : MqBase(rhs)
{
    rhs.m_interfaceName.clear();
}

MqInterfaceUser& MqInterfaceUser::operator=(MqInterfaceUser&& rhs) noexcept
{
    if (&rhs != this)
    {
        if (hasClosableMessageQueue() && m_isInitialized)
        {
            closeMessageQueue();
        }
        *static_cast<MqBase*>(this) = static_cast<MqBase>(rhs);

        rhs.m_interfaceName.clear();
    }

    return *this;
}

MqInterfaceUser::~MqInterfaceUser() noexcept
{
    if (hasClosableMessageQueue() && m_isInitialized)
    {
        closeMessageQueue();
    }
}

MqInterfaceCreator::MqInterfaceCreator(const std::string& name, const long maxMessages, const long messageSize) noexcept
    : MqBase(name, maxMessages, messageSize)
{
    // @todo set umask to 0 to get 0666 permissions, create extra users for
    // daemon and applications later
    umask(0);

    // check if the mq is still there (e.g. because of no proper termination
    // of the process)
    cleanupOutdatedMessageQueue(name);

    m_isInitialized = openMessageQueue(name, O_CREAT | O_RDWR);
}

MqInterfaceCreator::MqInterfaceCreator(MqInterfaceCreator&& rhs) noexcept
    : MqBase(rhs)
{
    rhs.m_interfaceName.clear();
}

MqInterfaceCreator& MqInterfaceCreator::operator=(MqInterfaceCreator&& rhs) noexcept
{
    if (&rhs != this)
    {
        if (hasClosableMessageQueue())
        {
            closeMessageQueue();
            unlinkMessageQueue();
        }
        *static_cast<MqBase*>(this) = static_cast<MqBase>(rhs);

        rhs.m_interfaceName.clear();
    }

    return *this;
}

MqInterfaceCreator::~MqInterfaceCreator() noexcept
{
    if (hasClosableMessageQueue())
    {
        closeMessageQueue();
        unlinkMessageQueue();
    }
}

MqRuntimeInterface::MqRuntimeInterface(const std::string& roudiName,
                                       const std::string& appName,
                                       const units::Duration roudiWaitingTimeout) noexcept
    : m_appName(appName)
    , m_AppMqInterface(appName)
    , m_RoudiMqInterface(roudiName)
{
    posix::Timer timer(roudiWaitingTimeout);

    enum class RegState
    {
        WAIT_FOR_ROUDI,
        SEND_REGISTER_REQUEST,
        WAIT_FOR_REGISTER_ACK,
        FINISHED
    };

    int64_t transmissionTimestamp{0};
    auto regState = RegState::WAIT_FOR_ROUDI;
    while (!timer.hasExpiredComparedToCreationTime() && regState != RegState::FINISHED)
    {
        if (!m_RoudiMqInterface.mqMapsToFile() || !m_RoudiMqInterface.isInitialized())
        {
            LogDebug() << "reopen RouDi mqueue!";
            m_RoudiMqInterface.reopen();
            regState = RegState::WAIT_FOR_ROUDI;
        }

        switch (regState)
        {
        case RegState::WAIT_FOR_ROUDI:
        {
            waitForRoudi(timer);
            if (m_RoudiMqInterface.isInitialized())
            {
                regState = RegState::SEND_REGISTER_REQUEST;
            }
            break;
        }
        case RegState::SEND_REGISTER_REQUEST:
        {
            using namespace units;
            using namespace std::chrono;
            auto timestamp = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
            while (transmissionTimestamp == timestamp)
            {
                timestamp = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
            }
            transmissionTimestamp = timestamp;

            // send MqMessageType::REG to RouDi
            bool successfullySent =
                m_RoudiMqInterface.timedSend({mqMessageTypeToString(MqMessageType::REG),
                                              m_appName,
                                              std::to_string(getpid()),
                                              std::to_string(posix::PosixUser::getUserOfCurrentProcess().getID()),
                                              std::to_string(transmissionTimestamp)},
                                             100_ms);

            if (successfullySent)
            {
                regState = RegState::WAIT_FOR_REGISTER_ACK;
            }
            break;
        }
        case RegState::WAIT_FOR_REGISTER_ACK:
            if (waitForRegAck(transmissionTimestamp) == RegAckResult::SUCCESS)
            {
                regState = RegState::FINISHED;
            }
            break;
        case RegState::FINISHED:
            // nothing to do, move along
            break;
        }
    }

    if (regState != RegState::FINISHED)
    {
        /// @todo: a temporary mechanism to allow a cleanup of the app message queue
        m_AppMqInterface.~MqInterfaceCreator();
    }
    switch (regState)
    {
    case RegState::WAIT_FOR_ROUDI:
        errorHandler(Error::kMQ_INTERFACE__REG_ROUDI_NOT_AVAILABLE);
        break;
    case RegState::SEND_REGISTER_REQUEST:
        errorHandler(Error::kMQ_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_MQ);
        break;
    case RegState::WAIT_FOR_REGISTER_ACK:
        errorHandler(Error::kMQ_INTERFACE__REG_ACK_NO_RESPONSE);
        break;
    case RegState::FINISHED:
        // nothing to do, move along
        break;
    }
}

bool MqRuntimeInterface::sendKeepalive() noexcept
{
    return m_RoudiMqInterface.send({mqMessageTypeToString(MqMessageType::KEEPALIVE), m_appName});
}

std::string MqRuntimeInterface::getShmBaseAddr() const noexcept
{
    return m_shmBaseAddr;
}

std::string MqRuntimeInterface::getSegmentManagerAddr() const noexcept
{
    return m_segmentManager;
}

bool MqRuntimeInterface::sendRequestToRouDi(const MqMessage& msg, MqMessage& answer) noexcept
{
    if (!m_RoudiMqInterface.send(msg))
    {
        LogError() << "Could not send request via RouDi messagequeue interface.\n";
        return false;
    }

    if (!m_AppMqInterface.receive(answer))
    {
        LogError() << "Could not receive request via App messagequeue interface.\n";
        return false;
    }

    return true;
}

bool MqRuntimeInterface::sendMessageToRouDi(const MqMessage& msg) noexcept
{
    if (!m_RoudiMqInterface.send(msg))
    {
        LogError() << "Could not send message via RouDi messagequeue interface.\n";
        return false;
    }
    return true;
}

size_t MqRuntimeInterface::getShmTopicSize() noexcept
{
    return m_shmTopicSize;
}

void MqRuntimeInterface::waitForRoudi(posix::Timer& timer) noexcept
{
    bool printWaitingWarning = true;
    bool printFoundMessage = false;
    while (!timer.hasExpiredComparedToCreationTime() && !m_RoudiMqInterface.isInitialized())
    {
        m_RoudiMqInterface.reopen();

        if (m_RoudiMqInterface.isInitialized())
        {
            LogDebug() << "RouDi mqueue found!";
            break;
        }

        if (printWaitingWarning)
        {
            LogDebug() << "RouDi not found - waiting ...";
            printWaitingWarning = false;
            printFoundMessage = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (printFoundMessage && m_RoudiMqInterface.isInitialized())
    {
        LogWarn() << "... RouDi found.";
    }
}

MqRuntimeInterface::RegAckResult MqRuntimeInterface::waitForRegAck(int64_t transmissionTimestamp) noexcept
{
    // wait for the register ack from the RouDi daemon. If we receive another response we do a retry
    // @todo if message queues are properly setup and cleaned up, always the expected REG_ACK should be received here,
    // so if not this issue should be passed to the error handling later
    constexpr size_t MAX_RETRY_COUNT = 3;
    size_t retryCounter = 0;
    while (retryCounter++ < MAX_RETRY_COUNT)
    {
        MqMessage receiveBuffer;
        // wait for MqMessageType::REG_ACK from RouDi for 1 seconds
        if (m_AppMqInterface.timedReceive(1000, receiveBuffer))
        {
            std::string cmd = receiveBuffer.getElementAtIndex(0);

            if (stringToMqMessageType(cmd.c_str()) == MqMessageType::REG_ACK)
            {
                constexpr uint32_t REGISTER_ACK_PARAMETERS = 6;
                if (receiveBuffer.getNumberOfElements() != REGISTER_ACK_PARAMETERS)
                {
                    errorHandler(Error::kMQ_INTERFACE__REG_ACK_INVALIG_NUMBER_OF_PARAMS);
                }

                // read out the shared memory base address and save it
                m_shmBaseAddr = receiveBuffer.getElementAtIndex(1);
                m_shmTopicSize = strtoull(receiveBuffer.getElementAtIndex(2).c_str(), nullptr, 10);
                m_segmentManager = receiveBuffer.getElementAtIndex(3);

                int64_t receivedTimestamp;
                cxx::convert::fromString(receiveBuffer.getElementAtIndex(4).c_str(), receivedTimestamp);
                cxx::convert::fromString(receiveBuffer.getElementAtIndex(5).c_str(), m_segmentId);
                if (transmissionTimestamp == receivedTimestamp)
                {
                    return RegAckResult::SUCCESS;
                }
                else
                {
                    LogWarn() << "Received a REG_ACK with an outdated timestamp!";
                }
            }
            else
            {
                LogError() << "Wrong response received " << receiveBuffer.getMessage();
            }
        }
    }

    return RegAckResult::TIMEOUT;
}

uint64_t MqRuntimeInterface::getSegmentId() const noexcept
{
    return m_segmentId;
}

} // namespace runtime
} // namespace iox
