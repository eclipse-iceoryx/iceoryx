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

#include "eth/eth_reader.hpp"
#include "eth/eth_context.hpp"
#include "internal/log/logging.hpp"

iox::eth::ethDataReader::ethDataReader(IdString serviceId, IdString instanceId, IdString eventId) noexcept
    : m_serviceId(serviceId)
    , m_instanceId(instanceId)
    , m_eventId(eventId)
{
    LogDebug() << "[ethDataReader] Created ethDataReader.";
}

iox::eth::ethDataReader::~ethDataReader()
{
    LogDebug() << "[ethDataReader] Destroyed ethDataReader.";
}

void iox::eth::ethDataReader::connect() noexcept
{
    // if (!m_isConnected.load(std::memory_order_relaxed))
    // {
    //     auto topicString =
    //         "/" + std::string(m_serviceId) + "/" + std::string(m_instanceId) + "/" + std::string(m_eventId);
    //     auto topic = ::eth::topic::Topic<Mempool::Chunk>(CycloneContext::getParticipant(), topicString);
    //     auto subscriber = ::eth::sub::Subscriber(CycloneContext::getParticipant());

    //     auto qos = ::eth::sub::qos::DataReaderQos();
    //     qos << ::eth::core::policy::History::KeepAll();

    //     m_impl = ::eth::sub::DataReader<Mempool::Chunk>(subscriber, topic, qos);

    //     LogDebug() << "[ethDataReader] Connected to topic: " << topicString;

    //     m_isConnected.store(true, std::memory_order_relaxed);
    // }
}

iox::cxx::optional<uint64_t> iox::eth::ethDataReader::peekNextSize()
{
    // ensure to only read sample - do not take
    // auto readSamples = m_impl.select().max_samples(1u).state(::eth::sub::status::SampleState::any()).read();

    // if (readSamples.length() > 0)
    // {
    //     auto nextSample = readSamples.begin();
    //     auto nextSampleSize = nextSample->data().payload().size();

    //     // Ignore samples with no payload
    //     if (nextSampleSize != 0)
    //     {
    //         return iox::cxx::optional<uint64_t>(static_cast<uint64_t>(nextSampleSize));
    //     }
    // }

    // no valid samples available
    return iox::cxx::nullopt_t();
}

iox::cxx::expected<iox::eth::DataReaderError> iox::eth::ethDataReader::takeNext(uint8_t* const buffer,
                                                                                    const uint64_t& bufferSize)
{
    // // validation checks
    // if (!m_isConnected.load())
    // {
    //     return iox::cxx::error<iox::eth::DataReaderError>(iox::eth::DataReaderError::NOT_CONNECTED);
    // }
    // if (buffer == nullptr)
    // {
    //     return iox::cxx::error<iox::eth::DataReaderError>(iox::eth::DataReaderError::INVALID_RECV_BUFFER);
    // }

    // // take next sample and copy into buffer
    // auto takenSamples = m_impl.select().max_samples(1u).state(::eth::sub::status::SampleState::any()).take();
    // if (takenSamples.length() == 0)
    // {
    //     // no samples available
    //     return iox::cxx::success<>();
    // }

    // // valid size
    // auto nextSample = takenSamples.begin();
    // auto sampleSize = nextSample->data().payload().size();
    // if (sampleSize == 0)
    // {
    //     return iox::cxx::error<iox::eth::DataReaderError>(iox::eth::DataReaderError::INVALID_DATA);
    // }
    // if (bufferSize < sampleSize)
    // {
    //     // provided buffer is too small.
    //     return iox::cxx::error<iox::eth::DataReaderError>(iox::eth::DataReaderError::RECV_BUFFER_TOO_SMALL);
    // }

    // // copy data into the provided buffer
    // auto bytes = nextSample->data().payload().data();
    // std::copy(bytes, bytes + sampleSize, buffer);

    return iox::cxx::success<>();
}

iox::cxx::expected<uint64_t, iox::eth::DataReaderError> iox::eth::ethDataReader::take(
    uint8_t* const buffer, const uint64_t& bufferSize, const iox::cxx::optional<uint64_t>& maxSamples)
{
    // if (!m_isConnected.load())
    // {
    //     return iox::cxx::error<iox::eth::DataReaderError>(iox::eth::DataReaderError::NOT_CONNECTED);
    // }
    // if (buffer == nullptr)
    // {
    //     return iox::cxx::error<iox::eth::DataReaderError>(iox::eth::DataReaderError::INVALID_RECV_BUFFER);
    // }

    // // get size of the sample
    // auto peekResult = peekNextSize();
    // if (peekResult.has_value())
    // {
    //     uint64_t sampleSize = peekResult.value();
    //     if (sampleSize == 0)
    //     {
    //         return iox::cxx::error<iox::eth::DataReaderError>(iox::eth::DataReaderError::INVALID_DATA);
    //     }
    //     if (bufferSize < sampleSize)
    //     {
    //         // Provided buffer is too small.
    //         return iox::cxx::error<iox::eth::DataReaderError>(iox::eth::DataReaderError::RECV_BUFFER_TOO_SMALL);
    //     }

    //     // take maximum amount possible up to the cap: maxSamples
    //     auto bufferCapacity = bufferSize / sampleSize;

    //     auto numToTake = bufferCapacity;
    //     if (maxSamples.has_value())
    //     {
    //         if (bufferCapacity > maxSamples.value())
    //         {
    //             numToTake = maxSamples.value();
    //         }
    //     }
    //     auto samples = m_impl.select()
    //                        .max_samples(static_cast<uint32_t>(numToTake))
    //                        .state(::eth::sub::status::SampleState::any())
    //                        .take();

    //     // copy data into the provided buffer
    //     uint64_t numSamplesBuffered = 0u;
    //     if (samples.length() > 0)
    //     {
    //         // do copy
    //         uint64_t cursor = 0; // Tracks the position in the buffer to write next sample.
    //         for (const auto& sample : samples)
    //         {
    //             auto bytes = sample.data().payload().data();
    //             std::copy(bytes, bytes + sampleSize, &buffer[cursor]);
    //             cursor += sampleSize;
    //         }
    //         numSamplesBuffered = cursor / sampleSize;
    //     }
    //     return iox::cxx::success<uint64_t>(numSamplesBuffered);
    // }
    // else
    // {
    //     return iox::cxx::success<uint64_t>(0u);
    // }
}

iox::eth::IdString iox::eth::ethDataReader::getServiceId() const noexcept
{
    return m_serviceId;
}

iox::eth::IdString iox::eth::ethDataReader::getInstanceId() const noexcept
{
    return m_instanceId;
}

iox::eth::IdString iox::eth::ethDataReader::getEventId() const noexcept
{
    return m_eventId;
}
