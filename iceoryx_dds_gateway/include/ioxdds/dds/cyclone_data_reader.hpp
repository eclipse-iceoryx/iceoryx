#pragma once

#include "ioxdds/dds/data_reader.hpp"

namespace iox {
namespace dds {

class CycloneDataReader : DataReader<CycloneDataReader>
{
public:
    CycloneDataReader(IdString serviceId, IdString instanceId, IdString eventId) noexcept;
};

} // namespace dds
} // namespace iox
