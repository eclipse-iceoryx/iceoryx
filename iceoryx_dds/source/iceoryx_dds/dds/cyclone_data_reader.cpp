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

#include "iceoryx_dds/dds/cyclone_data_reader.hpp"
#include "iceoryx_dds/dds/cyclone_context.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"

iox::dds::CycloneDataReader::CycloneDataReader(const capro::IdString_t serviceId,
                                               const capro::IdString_t instanceId,
                                               const capro::IdString_t eventId) noexcept
    : m_serviceId(serviceId)
    , m_instanceId(instanceId)
    , m_eventId(eventId)
{
    LogDebug() << "[CycloneDataReader] Created CycloneDataReader.";
}

iox::dds::CycloneDataReader::~CycloneDataReader()
{
    LogDebug() << "[CycloneDataReader] Destroyed CycloneDataReader.";
}

void iox::dds::CycloneDataReader::connect() noexcept
{
    if (!m_isConnected.load(std::memory_order_relaxed))
    {
        auto topicString =
            "/" + std::string(m_serviceId) + "/" + std::string(m_instanceId) + "/" + std::string(m_eventId);
        auto topic = ::dds::topic::Topic<Mempool::Chunk>(CycloneContext::getParticipant(), topicString);
        auto subscriber = ::dds::sub::Subscriber(CycloneContext::getParticipant());

        auto qos = ::dds::sub::qos::DataReaderQos();
        qos << ::dds::core::policy::History::KeepAll();

        m_impl = ::dds::sub::DataReader<Mempool::Chunk>(subscriber, topic, qos);

        LogDebug() << "[CycloneDataReader] Connected to topic: " << topicString;

        m_isConnected.store(true, std::memory_order_relaxed);
    }
}

iox::cxx::optional<uint32_t> iox::dds::CycloneDataReader::peekNextSize()
{
    // ensure to only read sample - do not take
    auto readSamples = m_impl.select().max_samples(1U).state(::dds::sub::status::SampleState::any()).read();

    if (readSamples.length() > 0)
    {
        auto nextSample = readSamples.begin();
        auto nextSampleSize = nextSample->data().payload().size();

        // Ignore samples with no payload
        if (nextSampleSize != 0)
        {
            return iox::cxx::optional<uint32_t>(static_cast<uint32_t>(nextSampleSize));
        }
    }

    // no valid samples available
    return iox::cxx::nullopt_t();
}

iox::cxx::optional<iox::dds::IoxChunkDatagramHeader> iox::dds::CycloneDataReader::peekNextIoxChunkDatagramHeader()
{
    // ensure to only read sample - do not take
    auto readSamples = m_impl.select().max_samples(1U).state(::dds::sub::status::SampleState::any()).read();

    constexpr iox::cxx::nullopt_t NO_VALID_SAMPLE_AVAILABLE;

    if (readSamples.length() == 0)
    {
        return NO_VALID_SAMPLE_AVAILABLE;
    }

    auto nextSample = readSamples.begin();
    auto& nextSamplePayload = nextSample->data().payload();
    auto nextSampleSize = nextSamplePayload.size();

    // Ignore samples with no payload
    if (nextSampleSize == 0)
    {
        return NO_VALID_SAMPLE_AVAILABLE;
    }

    // Ignore Invalid IoxChunkDatagramHeader
    if (nextSampleSize < sizeof(iox::dds::IoxChunkDatagramHeader))
    {
        auto log = LogError();
        log << "[CycloneDataReader] invalid sample size! Must be at least sizeof(IoxChunkDatagramHeader) = "
            << sizeof(iox::dds::IoxChunkDatagramHeader) << " but got " << nextSampleSize;
        if (nextSampleSize >= 1)
        {
            log << "! Potential datagram version is " << static_cast<uint16_t>(nextSamplePayload[0])
                << "! Dropped sample!";
        }
        return NO_VALID_SAMPLE_AVAILABLE;
    }

    if (nextSamplePayload[0] != iox::dds::IoxChunkDatagramHeader::DATAGRAM_VERSION)
    {
        LogError()
            << "[CycloneDataReader] received sample with incompatible IoxChunkDatagramHeader version! Dropped sample!";
        return NO_VALID_SAMPLE_AVAILABLE;
    }

    if (static_cast<iox::dds::Endianess>(nextSamplePayload[1]) != getEndianess())
    {
        LogError() << "[CycloneDataReader] received sample with incompatible endianess! Dropped sample!";
        return NO_VALID_SAMPLE_AVAILABLE;
    }

    iox::dds::IoxChunkDatagramHeader datagramHeader;

    std::memcpy(reinterpret_cast<uint8_t*>(&datagramHeader),
                nextSamplePayload.data(),
                sizeof(iox::dds::IoxChunkDatagramHeader));

    return datagramHeader;
}

bool iox::dds::CycloneDataReader::hasSamples()
{
    auto samples = m_impl.select().max_samples(1u).state(::dds::sub::status::SampleState::any()).read();
    return samples.length() > 0;
}

iox::cxx::expected<iox::dds::DataReaderError> iox::dds::CycloneDataReader::takeNext(uint8_t* const buffer,
                                                                                    const uint64_t& bufferSize)
{
    // validation checks
    if (!m_isConnected.load())
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::NOT_CONNECTED);
    }
    if (buffer == nullptr)
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::INVALID_RECV_BUFFER);
    }

    // take next sample and copy into buffer
    auto takenSamples = m_impl.select().max_samples(1U).state(::dds::sub::status::SampleState::any()).take();
    if (takenSamples.length() == 0)
    {
        // no samples available
        return iox::cxx::success<>();
    }

    // valid size
    auto nextSample = takenSamples.begin();
    auto sampleSize = nextSample->data().payload().size();
    if (sampleSize == 0)
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::INVALID_DATA);
    }
    if (bufferSize < sampleSize)
    {
        // provided buffer is too small.
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::RECV_BUFFER_TOO_SMALL);
    }

    // copy data into the provided buffer
    auto bytes = nextSample->data().payload().data();
    std::copy(bytes, bytes + sampleSize, buffer);

    return iox::cxx::success<>();
}

iox::cxx::expected<iox::dds::DataReaderError> iox::dds::CycloneDataReader::takeNext(
    const iox::dds::IoxChunkDatagramHeader datagramHeader, uint8_t* userHeaderBytes, uint8_t* userPayloadBytes)
{
    // validation checks
    if (!m_isConnected.load())
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::NOT_CONNECTED);
    }

    // take next sample and copy into buffer
    auto takenSamples = m_impl.select().max_samples(1U).state(::dds::sub::status::SampleState::any()).take();
    if (takenSamples.length() == 0)
    {
        // no samples available
        return iox::cxx::success<>();
    }

    // valid size
    auto nextSample = takenSamples.begin();
    auto samplePayload = nextSample->data().payload();
    auto sampleSize = samplePayload.size();
    if (sampleSize == 0)
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::INVALID_DATA);
    }
    if (sampleSize < sizeof(iox::dds::IoxChunkDatagramHeader))
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::INVALID_DATAGRAM_HEADER);
    }

    // it is assume that peekNextIoxChunkDatagramHeader was called beforehand and that the provided datagramHeader
    // belongs to this sample
    if (datagramHeader.userHeaderSize > 0
        && (datagramHeader.userHeaderId == iox::mepoo::ChunkHeader::NO_USER_HEADER || userHeaderBytes == nullptr))
    {
        return iox::cxx::error<iox::dds::DataReaderError>(
            iox::dds::DataReaderError::INVALID_BUFFER_PARAMETER_FOR_USER_HEADER);
    }
    if (datagramHeader.userPayloadSize > 0 && userHeaderBytes == nullptr)
    {
        return iox::cxx::error<iox::dds::DataReaderError>(
            iox::dds::DataReaderError::INVALID_BUFFER_PARAMETER_FOR_USER_PAYLOAD);
    }

    auto dataSize = sampleSize - sizeof(iox::dds::IoxChunkDatagramHeader);
    auto bufferSize = datagramHeader.userHeaderSize + datagramHeader.userPayloadSize;

    if (bufferSize != dataSize)
    {
        // provided buffer don't match
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::BUFFER_SIZE_MISSMATCH);
    }


    // copy data into the provided buffer

    if (userHeaderBytes)
    {
        auto bytes = &samplePayload.data()[sizeof(iox::dds::IoxChunkDatagramHeader)];
        std::memcpy(userHeaderBytes, bytes, datagramHeader.userHeaderSize);
    }

    if (userPayloadBytes)
    {
        auto bytes = &samplePayload.data()[sizeof(iox::dds::IoxChunkDatagramHeader) + datagramHeader.userHeaderSize];
        std::memcpy(userPayloadBytes, bytes, datagramHeader.userPayloadSize);
    }

    return iox::cxx::success<>();
}

iox::cxx::expected<uint64_t, iox::dds::DataReaderError> iox::dds::CycloneDataReader::take(
    uint8_t* const buffer, const uint64_t& bufferSize, const iox::cxx::optional<uint64_t>& maxSamples)
{
    if (!m_isConnected.load())
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::NOT_CONNECTED);
    }
    if (buffer == nullptr)
    {
        return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::INVALID_RECV_BUFFER);
    }

    // get size of the sample
    auto peekResult = peekNextSize();
    if (peekResult.has_value())
    {
        uint64_t sampleSize = peekResult.value();
        if (sampleSize == 0)
        {
            return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::INVALID_DATA);
        }
        if (bufferSize < sampleSize)
        {
            // Provided buffer is too small.
            return iox::cxx::error<iox::dds::DataReaderError>(iox::dds::DataReaderError::RECV_BUFFER_TOO_SMALL);
        }

        // take maximum amount possible up to the cap: maxSamples
        auto bufferCapacity = bufferSize / sampleSize;

        auto numToTake = bufferCapacity;
        if (maxSamples.has_value())
        {
            if (bufferCapacity > maxSamples.value())
            {
                numToTake = maxSamples.value();
            }
        }
        auto samples = m_impl.select()
                           .max_samples(static_cast<uint32_t>(numToTake))
                           .state(::dds::sub::status::SampleState::any())
                           .take();

        // copy data into the provided buffer
        uint64_t numSamplesBuffered = 0u;
        if (samples.length() > 0)
        {
            // do copy
            uint64_t cursor = 0; // Tracks the position in the buffer to write next sample.
            for (const auto& sample : samples)
            {
                auto bytes = sample.data().payload().data();
                std::copy(bytes, bytes + sampleSize, &buffer[cursor]);
                cursor += sampleSize;
            }
            numSamplesBuffered = cursor / sampleSize;
        }
        return iox::cxx::success<uint64_t>(numSamplesBuffered);
    }
    else
    {
        return iox::cxx::success<uint64_t>(0u);
    }
}

iox::capro::IdString_t iox::dds::CycloneDataReader::getServiceId() const noexcept
{
    return m_serviceId;
}

iox::capro::IdString_t iox::dds::CycloneDataReader::getInstanceId() const noexcept
{
    return m_instanceId;
}

iox::capro::IdString_t iox::dds::CycloneDataReader::getEventId() const noexcept
{
    return m_eventId;
}
