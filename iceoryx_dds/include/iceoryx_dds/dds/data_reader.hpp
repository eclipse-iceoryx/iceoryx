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

#ifndef IOX_DDS_DDS_DATA_READER_HPP
#define IOX_DDS_DDS_DATA_READER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

namespace iox
{
namespace dds
{
enum class DataReaderError : uint8_t
{
    INVALID_STATE,
    NOT_CONNECTED,
    INVALID_RECV_BUFFER,
    INVALID_DATA,
    RECV_BUFFER_TOO_SMALL
};

constexpr char DataReaderErrorString[][64] = {
    "NOT_CONNECTED", "INVALID_RECV_BUFFER", "INVALID_DATA", "RECV_BUFFER_TOO_SMALL"};

class DataReader
{
  public:
    ///
    /// @brief connect Connect the DataReader to the underlying DDS network.
    ///
    virtual void connect() noexcept = 0;

    ///
    /// @brief peekNextSize Get the size of the next sample if one is available.
    /// @return The size of the next sample if one is available.
    ///
    virtual iox::cxx::optional<uint32_t> peekNextSize() = 0;

    ///
    /// @brief hasSamples Checks if new samples ready to take.
    /// @return True if new samples available.
    ///
    virtual bool hasSamples() = 0;

    ///
    /// @brief take Take the next available sample from the DDS data space.
    /// @param buffer Receive buffer in which sample will be stored.
    /// @param bufferSize Size of the provided buffer.
    /// @return Error if unsuccessful.
    ///
    virtual iox::cxx::expected<DataReaderError> takeNext(uint8_t* const buffer, const uint64_t& bufferSize) = 0;


    ///
    /// @brief take Take up to a maximum number of samples from the DDS data space.
    /// @param buffer Receive buffer in which samples will be stored.
    /// @param bufferSize The size of the buffer (in bytes).
    /// @param maxSamples The maximum number of samples to request from the network.
    /// @return Number of samples taken if successful. Number of samples will be in the sange [0,maxSamples].
    ///
    /// @note Sample size must be known ahead of time & can be checked using @ref peekNextSize() .
    ///
    virtual iox::cxx::expected<uint64_t, DataReaderError>
    take(uint8_t* const buffer, const uint64_t& bufferSize, const iox::cxx::optional<uint64_t>& maxSamples) = 0;

    ///
    /// @brief getServiceId
    /// @return The ID of the service producing the bytes
    ///
    virtual capro::IdString_t getServiceId() const noexcept = 0;

    ///
    /// @brief getInstanceId
    /// @return The ID of the instance of the service producing the bytes
    ///
    virtual capro::IdString_t getInstanceId() const noexcept = 0;

    ///
    /// @brief getEventId
    /// @return The ID of the event producing the data
    ///
    virtual capro::IdString_t getEventId() const noexcept = 0;

  protected:
    DataReader() = default;
};
} // namespace dds
} // namespace iox

#endif // IOX_DDS_DDS_DATA_READER_HPP
