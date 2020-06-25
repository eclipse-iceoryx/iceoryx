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

#ifndef IOX_DDS_DDS_DATA_READER_HPP
#define IOX_DDS_DDS_DATA_READER_HPP

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/string.hpp"

namespace iox
{
namespace dds
{
using IdString = iox::cxx::string<100u>;

enum class DataReaderError : uint8_t
{
    NOT_CONNECTED,
    INVALID_RECV_BUFFER,
    INVALID_DATA
};

class DataReader
{
  public:
    ///
    /// @brief connect Connect the DataReader to the underlying DDS network.
    ///
    virtual void connect() noexcept = 0;

    ///
    /// @brief read Read as many available unread samples as possible from the DDS network.
    ///
    /// @param buffer Receive buffer in which samples will be stored.
    /// @param bufferSize The size of the buffer (in bytes).
    /// @param sampleSize The expected size of the samples (in bytes).
    /// @return Number of samples read if successful.
    ///
    /// @note Maximum reads in one call is calculated as bufferSize / sampleSize.
    ///
    virtual iox::cxx::expected<uint8_t, DataReaderError>
    read(uint8_t* const buffer, const uint64_t& bufferSize, const uint64_t& sampleSize) = 0;

    ///
    /// \brief read Read a specified number of unread samples from the DDS network.
    /// @param buffer Receive buffer in which samples will be stored.
    /// @param bufferSize The size of the buffer (in bytes).
    /// @param sampleSize The expected size of the samples (in bytes).
    /// \param numSamples The number of samples to request from the network.
    /// \return Number of samples read if successful. Number of samples will be in the sange [0,num].
    ///
    virtual iox::cxx::expected<uint8_t, DataReaderError>
    read(uint8_t* const buffer, const uint64_t& bufferSize, const uint64_t& sampleSize, const uint64_t& numSamples) = 0;

    ///
    /// @brief getServiceId
    /// @return The ID of the service producing the bytes
    ///
    virtual IdString getServiceId() const noexcept = 0;

    ///
    /// @brief getInstanceId
    /// @return The ID of the instance of the service producing the bytes
    ///
    virtual IdString getInstanceId() const noexcept = 0;

    ///
    /// @brief getEventId
    /// @return The ID of the event producing the data
    ///
    virtual IdString getEventId() const noexcept = 0;

  protected:
    DataReader() = default;
};

} // namespace dds
} // namespace iox

#endif // IOX_DDS_DDS_DATA_READER_HPP
