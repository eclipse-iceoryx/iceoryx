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

#ifndef IOX_DDS_DDS_CYCLONE_DATA_WRITER_HPP
#define IOX_DDS_DDS_CYCLONE_DATA_WRITER_HPP

#include "iceoryx_dds/dds/data_writer.hpp"

#include <dds/dds.hpp>
#include <Mempool_DCPS.hpp>


namespace iox
{
namespace dds
{
///
/// @brief Implementation of the DataWriter interface using the cyclonedds implementation.
///
class CycloneDataWriter : public iox::dds::DataWriter<CycloneDataWriter>
{
  public:
    CycloneDataWriter() = delete;
    CycloneDataWriter(const IdString serviceId, const IdString instanceId, const IdString eventId);
    virtual ~CycloneDataWriter();

    CycloneDataWriter(const CycloneDataWriter&) = delete;
    CycloneDataWriter& operator=(const CycloneDataWriter&) = delete;

    // Required for vector
    CycloneDataWriter(CycloneDataWriter&&) = default;
    CycloneDataWriter& operator=(CycloneDataWriter&&) = default;

    void connect() noexcept;
    void write(const uint8_t* const bytes, const uint64_t size) noexcept;
    std::string getServiceId() const noexcept;
    std::string getInstanceId() const noexcept;
    std::string getEventId() const noexcept;

  private:
    IdString m_serviceId{""};
    IdString m_instanceId{""};
    IdString m_eventId{""};

    ::dds::pub::Publisher m_publisher = ::dds::core::null;
    ::dds::topic::Topic<Mempool::Chunk> m_topic = ::dds::core::null;
    ::dds::pub::DataWriter<Mempool::Chunk> m_writer = ::dds::core::null;

    static ::dds::domain::DomainParticipant& getParticipant() noexcept;
};

} // namespace dds
} // namespace iox

#endif // IOX_DDS_DDS_CYCLONE_DATA_WRITER_HPP
