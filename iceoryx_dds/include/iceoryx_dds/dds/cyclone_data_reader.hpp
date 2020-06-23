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

#ifndef IOX_DDS_DDS_CYCLONE_DATA_READER_HPP
#define IOX_DDS_DDS_CYCLONE_DATA_READER_HPP

#include "iceoryx_dds/dds/data_reader.hpp"

#include <atomic>
#include <Mempool_DCPS.hpp>
#include <dds/dds.hpp>

namespace iox
{
namespace dds
{
/// @note Implementation coming soon.
class CycloneDataReader : public DataReader
{
  public:
    CycloneDataReader() = delete;
    CycloneDataReader(IdString serviceId, IdString instanceId, IdString eventId) noexcept;
    virtual ~CycloneDataReader();

    void connect() noexcept override;
    iox::cxx::expected<uint8_t, DataReaderError> read(uint8_t* buffer, const uint64_t& size) override;
    IdString getServiceId() const noexcept override;
    IdString getInstanceId() const noexcept override;
    IdString getEventId() const noexcept override;

  private:
    IdString m_serviceId{""};
    IdString m_instanceId{""};
    IdString m_eventId{""};

    ::dds::sub::Subscriber m_subscriber = ::dds::core::null;
    ::dds::topic::Topic<Mempool::Chunk> m_topic = ::dds::core::null;
    ::dds::sub::DataReader<Mempool::Chunk> m_reader = ::dds::core::null;

    std::atomic_bool m_isConnected{false};

};

} // namespace dds
} // namespace iox

#endif // IOX_DDS_DDS_CYCLONE_DATA_READER_HPP
