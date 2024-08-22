// Copyright 2024, Eclipse Foundation and the iceoryx contributors. All rights reserved.
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
#ifndef IOX_HOOFS_POSIX_IPC_MESSAGE_QUEUE_INL
#define IOX_HOOFS_POSIX_IPC_MESSAGE_QUEUE_INL

#include "iox/duration.hpp"
#include "iox/message_queue.hpp"
#include "iox/not_null.hpp"
#include "iox/posix_call.hpp"

namespace iox
{
template <typename Type, MessageQueue::Termination Terminator>
expected<void, PosixIpcChannelError>
MessageQueue::timedSendImpl(not_null<const Type*> msg, uint64_t msgSize, const units::Duration& timeout) const noexcept
{
    uint64_t msgSizeToSend = msgSize;
    if constexpr (Terminator == Termination::NULL_TERMINATOR)
    {
        msgSizeToSend += NULL_TERMINATOR_SIZE;
    }

    if (msgSizeToSend > static_cast<uint64_t>(m_attributes.mq_msgsize))
    {
        IOX_LOG(ERROR, "the message which should be sent to the message queue '" << m_name << "' is too long");
        return err(PosixIpcChannelError::MESSAGE_TOO_LONG);
    }

    timespec timeOut = timeout.timespec(units::TimeSpecReference::Epoch);
    auto mqCall = IOX_POSIX_CALL(mq_timedsend)(m_mqDescriptor, msg, static_cast<size_t>(msgSizeToSend), 1U, &timeOut)
                      .failureReturnValue(ERROR_CODE)
                      // don't use the suppressErrorMessagesForErrnos method since QNX used EINTR instead of ETIMEDOUT
                      .ignoreErrnos(TIMEOUT_ERRNO)
                      .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    if (mqCall->errnum == TIMEOUT_ERRNO)
    {
        return err(errnoToEnum(ETIMEDOUT));
    }

    return ok();
}

template <typename Type, MessageQueue::Termination Terminator>
expected<void, PosixIpcChannelError> MessageQueue::sendImpl(not_null<const Type*> msg, uint64_t msgSize) const noexcept
{
    uint64_t msgSizeToSend = msgSize;
    if constexpr (Terminator == Termination::NULL_TERMINATOR)
    {
        msgSizeToSend += NULL_TERMINATOR_SIZE;
    }

    if (msgSizeToSend > static_cast<uint64_t>(m_attributes.mq_msgsize))
    {
        IOX_LOG(ERROR, "the message which should be sent to the message queue '" << m_name << "' is too long");
        return err(PosixIpcChannelError::MESSAGE_TOO_LONG);
    }

    auto mqCall = IOX_POSIX_CALL(mq_send)(m_mqDescriptor, msg, static_cast<size_t>(msgSizeToSend), 1U)
                      .failureReturnValue(ERROR_CODE)
                      .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    if (mqCall->errnum == TIMEOUT_ERRNO)
    {
        return err(errnoToEnum(ETIMEDOUT));
    }

    return ok();
}

template <typename Type, MessageQueue::Termination Terminator>
expected<uint64_t, PosixIpcChannelError>
MessageQueue::timedReceiveImpl(not_null<Type*> msg, uint64_t maxMsgSize, const units::Duration& timeout) const noexcept
{
    timespec timeOut = timeout.timespec(units::TimeSpecReference::Epoch);
    auto mqCall =
        IOX_POSIX_CALL(mq_timedreceive)(m_mqDescriptor, msg, static_cast<size_t>(maxMsgSize), nullptr, &timeOut)
            .failureReturnValue(ERROR_CODE)
            // don't use the suppressErrorMessagesForErrnos method since QNX used EINTR instead of ETIMEDOUT
            .ignoreErrnos(TIMEOUT_ERRNO)
            .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    if (mqCall->errnum == TIMEOUT_ERRNO)
    {
        return err(errnoToEnum(ETIMEDOUT));
    }

    return receiveVerification<Type, Terminator>(msg, static_cast<uint64_t>(mqCall->value));
}

template <typename Type, MessageQueue::Termination Terminator>
expected<uint64_t, PosixIpcChannelError> MessageQueue::receiveImpl(not_null<Type*> msg,
                                                                   uint64_t maxMsgSize) const noexcept
{
    auto mqCall = IOX_POSIX_CALL(mq_receive)(m_mqDescriptor, msg, static_cast<size_t>(maxMsgSize), nullptr)
                      .failureReturnValue(ERROR_CODE)
                      .evaluate();

    if (mqCall.has_error())
    {
        return err(errnoToEnum(mqCall.error().errnum));
    }

    if (mqCall->errnum == TIMEOUT_ERRNO)
    {
        return err(errnoToEnum(ETIMEDOUT));
    }

    return receiveVerification<Type, Terminator>(msg, static_cast<uint64_t>(mqCall->value));
}

template <uint64_t N>
expected<void, PosixIpcChannelError> MessageQueue::send(const iox::string<N>& buf) const noexcept
{
    return sendImpl<char, Termination::NULL_TERMINATOR>(buf.c_str(), buf.size());
}

template <uint64_t N>
expected<void, PosixIpcChannelError> MessageQueue::timedSend(const iox::string<N>& buf,
                                                             const units::Duration& timeout) const noexcept
{
    return timedSendImpl<char, Termination::NULL_TERMINATOR>(buf.c_str(), buf.size(), timeout);
}

template <uint64_t N>
expected<void, PosixIpcChannelError> MessageQueue::receive(iox::string<N>& buf) const noexcept
{
    static_assert(N <= MAX_MESSAGE_SIZE, "Size exceeds transmission limit!");

    auto result = expected<uint64_t, PosixIpcChannelError>(in_place, uint64_t(0));
    buf.unsafe_raw_access([&](auto* str, const auto info) -> uint64_t {
        result = receiveImpl<char, Termination::NULL_TERMINATOR>(str, info.total_size);
        if (result.has_error())
        {
            return 0;
        }
        return result.value();
    });
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok();
}

template <uint64_t N>
expected<void, PosixIpcChannelError> MessageQueue::timedReceive(iox::string<N>& buf,
                                                                const units::Duration& timeout) const noexcept
{
    static_assert(N <= MAX_MESSAGE_SIZE, "Size exceeds transmission limit!");

    auto result = expected<uint64_t, PosixIpcChannelError>(in_place, uint64_t(0));
    buf.unsafe_raw_access([&](auto* str, const auto info) -> uint64_t {
        result = timedReceiveImpl<char, Termination::NULL_TERMINATOR>(str, info.total_size, timeout);
        if (result.has_error())
        {
            return 0;
        }
        return result.value();
    });
    if (result.has_error())
    {
        return err(result.error());
    }
    return ok();
}

template <typename Type, MessageQueue::Termination Terminator>
expected<uint64_t, PosixIpcChannelError> MessageQueue::receiveVerification(not_null<Type*> msg,
                                                                           uint64_t msgLenght) const noexcept
{
    if constexpr (Terminator == Termination::NULL_TERMINATOR)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (msg[msgLenght - NULL_TERMINATOR_SIZE] != 0)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            msg[0] = 0;
            return err(PosixIpcChannelError::INTERNAL_LOGIC_ERROR);
        }
        return ok<uint64_t>(msgLenght - NULL_TERMINATOR_SIZE);
    }

    return ok<uint64_t>(msgLenght);
}
} // namespace iox

#endif
