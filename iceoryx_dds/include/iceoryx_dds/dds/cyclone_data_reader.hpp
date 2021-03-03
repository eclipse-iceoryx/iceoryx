// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_DDS_DDS_CYCLONE_DATA_READER_HPP
#define IOX_DDS_DDS_CYCLONE_DATA_READER_HPP

#include "iceoryx_dds/dds/data_reader.hpp"

#include <Mempool_DCPS.hpp>
#include <atomic>
#include <dds/dds.hpp>

namespace iox
{
namespace dds
{
/// @brief Implementation of the DataReader abstraction using the cyclonedds implementation.
class CycloneDataReader : public DataReader
{
  public:
    CycloneDataReader() = delete;

    /// @brief Constructor to set cyclone data reader object from given IDs
    /// @param[in] serviceId ID of the service
    /// @param[in] instanceId ID of the instance of the service
    /// @param[in] eventId ID of the event
    CycloneDataReader(const capro::IdString_t serviceId,
                      const capro::IdString_t instanceId,
                      const capro::IdString_t eventId) noexcept;

    virtual ~CycloneDataReader();

    CycloneDataReader(const CycloneDataReader&) = delete;
    CycloneDataReader& operator=(const CycloneDataReader&) = delete;
    CycloneDataReader(CycloneDataReader&&) = delete;
    CycloneDataReader& operator=(CycloneDataReader&&) = delete;

    /// @brief Connect cylcone data reader to the underlying DDS network
    void connect() noexcept override;

    /// @brief Get the size of the next sample if one is available
    /// @return an optional pointer to the size of the next sample if one is available, cxx::nullopt_t if value is not
    /// initialized
    iox::cxx::optional<uint32_t> peekNextSize() override;

    /// @brief Check if new samples are ready to take
    /// @return true if there are samples available, otherwise false
    bool hasSamples() override;

    /// @brief Take the next available sample from the DDS data space.
    /// @param[in] buffer Receive buffer in which sample will be stored.
    /// @param[in] bufferSize Size of the provided buffer.
    /// @return Error if reader is not connected, buffer is too small, data or buffer is invalid
    iox::cxx::expected<DataReaderError> takeNext(uint8_t* const buffer, const uint64_t& bufferSize) override;

    /// @brief Take up to a maximum number of samples from the DDS data space.
    /// @param[in] buffer Receive buffer in which samples will be stored.
    /// @param[in] bufferSize The size of the buffer (in bytes).
    /// @param[in] maxSamples The maximum number of samples to request from the network.
    /// @return Number of samples taken if successful. Number of samples will be in the range [0,maxSamples].
    iox::cxx::expected<uint64_t, DataReaderError>
    take(uint8_t* const buffer, const uint64_t& bufferSize, const iox::cxx::optional<uint64_t>& maxSamples) override;

    /// @brief Get ID of the service
    capro::IdString_t getServiceId() const noexcept override;

    /// @brief Get ID of the instance
    capro::IdString_t getInstanceId() const noexcept override;

    /// @brief Get ID of the event
    capro::IdString_t getEventId() const noexcept override;

  private:
    capro::IdString_t m_serviceId{""};
    capro::IdString_t m_instanceId{""};
    capro::IdString_t m_eventId{""};

    ::dds::sub::DataReader<Mempool::Chunk> m_impl = ::dds::core::null;

    std::atomic_bool m_isConnected{false};
};

} // namespace dds
} // namespace iox

#endif // IOX_DDS_DDS_CYCLONE_DATA_READER_HPP
