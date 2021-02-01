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

#pragma once

#ifndef IOX_POSH_POPO_SAMPLE_DELETER_HPP
#define IOX_POSH_POPO_SAMPLE_DELETER_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"

namespace iox
{
namespace popo
{
///
/// @brief The SampleDeleter struct is a custom deleter in functor form which releases loans to a sample's
/// underlying memory chunk via the corresponding port..
/// Each port should create its own instance of this deleter struct.
///

/// @todo can we couple this to the lifetime of portdata instead of port?
template <typename Port>
struct SampleDeleter
{
  public:
    SampleDeleter(Port& port)
        : m_port(&port)
    {
        // m_portData = port.getMembers();
    }

    template <typename T>
    void operator()(T* const ptr) const
    {
        auto header = iox::mepoo::ChunkHeader::fromPayload(ptr);
        m_port->releaseChunk(header);
        // Port(m_portData).releaseChunk(header); //would be preferable
    }

  private:
    Port* m_port;
    // PortData* m_portData; // we only rely on the port object stored in shared memory(!)
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_SAMPLE_DELETER_HPP
