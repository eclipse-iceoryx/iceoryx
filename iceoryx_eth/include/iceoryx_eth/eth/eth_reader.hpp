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

#ifndef IOX_eth_eth_CYCLONE_DATA_READER_HPP
#define IOX_eth_eth_CYCLONE_DATA_READER_HPP

#include "eth/data_reader.hpp"


namespace iox
{
namespace eth
{
///
/// @brief Implementation of the DataReader abstraction using the eth implementation.
///
class ethDataReader : public DataReader
{
  public:
    ethDataReader() = delete;
    ethDataReader(IdString serviceId, IdString instanceId, IdString eventId) noexcept;
    virtual ~ethDataReader();

    ethDataReader(const ethDataReader&) = delete;
    ethDataReader& operator=(const ethDataReader&) = delete;
    ethDataReader(ethDataReader&&) = delete;
    ethDataReader& operator=(ethDataReader&&) = delete;

    void connect() noexcept override;

    iox::cxx::optional<uint64_t> peekNextSize() override;

    iox::cxx::expected<DataReaderError> takeNext(uint8_t* const buffer, const uint64_t& bufferSize) override;

    iox::cxx::expected<uint64_t, DataReaderError>
    take(uint8_t* const buffer, const uint64_t& bufferSize, const iox::cxx::optional<uint64_t>& maxSamples) override;

    IdString getServiceId() const noexcept override;
    IdString getInstanceId() const noexcept override;
    IdString getEventId() const noexcept override;

  private:
    IdString m_serviceId{""};
    IdString m_instanceId{""};
    IdString m_eventId{""};

    

    //std::atomic_bool m_isConnected{false};
};

} // namespace eth
} // namespace iox

#endif // IOX_eth_eth_CYCLONE_DATA_READER_HPP
