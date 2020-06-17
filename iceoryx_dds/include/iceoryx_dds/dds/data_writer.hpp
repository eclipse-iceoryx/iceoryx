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

#ifndef IOX_DDS_DDS_DATA_WRITER_HPP
#define IOX_DDS_DDS_DATA_WRITER_HPP

#include "iceoryx_utils/cxx/string.hpp"

#include <cstdint>

namespace iox
{
namespace dds
{
using IdString = iox::cxx::string<100>;

///
/// @brief Interface for DDS Data Writers.
/// The interface provides the minimum functionality required for posh-dds gateway implementations.
///
template <typename Impl>
class DataWriter
{
  public:
    virtual ~DataWriter() = default;

    ///
    /// @brief connect Connect the DataWriter to the underlying DDS network.
    ///
    void connect() noexcept;

    ///
    /// @brief write Write the provided bytes on the DDS network on the topic: serviceId/instanceId/eventId
    /// @param bytes
    /// @param size
    ///
    void write(const uint8_t* const bytes, const uint64_t size) noexcept;

    ///
    /// @brief getServiceId
    /// @return The ID of the service producing the bytes
    ///
    IdString getServiceId() const noexcept;

    ///
    /// @brief getInstanceId
    /// @return The ID of the instance of the service producing the bytes
    ///
    IdString getInstanceId() const noexcept;

    ///
    /// @brief getEventId
    /// @return The ID of the event producing the data
    ///
    IdString getEventId() const noexcept;

  private:
    DataWriter() = default;
    friend Impl;
};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/dds/data_writer.inl"

#endif // IOX_DDS_DDS_DATA_WRITER_HPP
