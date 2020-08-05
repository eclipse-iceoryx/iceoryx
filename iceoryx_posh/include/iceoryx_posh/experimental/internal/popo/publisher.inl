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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_INL
#define IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_INL

#include "iceoryx_posh/experimental/popo/publisher.hpp"

#include <iostream>

namespace iox
{
namespace popo
{

template<typename T>
using chunk_t = T*;

using uid_t = uint64_t;

template<typename T, typename sender_port_t>
inline uid_t
Publisher<T, sender_port_t>::uid() const noexcept
{
    std::cout << "uid()" << std::endl;
    return 0u;
}

template<typename T, typename sender_port_t>
inline cxx::expected<chunk_t<T>, AllocationError>
Publisher<T, sender_port_t>::allocate() const noexcept
{
    std::cout << "allocate()" << std::endl;
    uint8_t* buf = new uint8_t[sizeof (T)];
    return iox::cxx::success<chunk_t>(reinterpret_cast<chunk_t>(buf));
}

template<typename T, typename sender_port_t>
cxx::expected<AllocationError>
Publisher<T, sender_port_t>::allocate(cxx::function_ref<void(chunk_t&) noexcept> f) const noexcept
{
    std::cout << "allocate(cxx::function_ref<chunk_t&>)" << std::endl;
    uint8_t* buf = new uint8_t[sizeof (T)];
    auto chunk = reinterpret_cast<chunk_t>(buf);
    f(chunk);
    return cxx::success<>();
};

template<typename T, typename sender_port_t>
inline void
Publisher<T, sender_port_t>::release(chunk_t&& chunk) const noexcept
{
    std::cout << "release()" << std::endl;
}

template<typename T, typename sender_port_t>
inline void
Publisher<T, sender_port_t>::publish(chunk_t&& chunk) const noexcept
{
    std::cout << "publish()" << std::endl;
}

template<typename T, typename sender_port_t>
inline void
Publisher<T, sender_port_t>::publishCopyOf(const T& val) const noexcept
{
    std::cout << "publishCopyOf()" << std::endl;
}

template<typename T, typename sender_port_t>
inline cxx::expected<ChunkRecallError>
Publisher<T, sender_port_t>::previous() const noexcept
{
    std::cout << "previous()" << std::endl;
}

template<typename T, typename sender_port_t>
inline void
Publisher<T, sender_port_t>::offer() noexcept
{
    std::cout << "offer()" << std::endl;
}

template<typename T, typename sender_port_t>
inline void
Publisher<T, sender_port_t>::stopOffer() noexcept
{
    std::cout << "stopOffer()" << std::endl;
}

template<typename T, typename sender_port_t>
inline bool
Publisher<T, sender_port_t>::isOffered() const noexcept
{
    std::cout << "isOffered()" << std::endl;
}

template<typename T, typename sender_port_t>
inline bool
Publisher<T, sender_port_t>::hasSubscribers() const noexcept
{
    std::cout << "hasSubscribers()" << std::endl;
}

} // namespace popo
} // namespace iox

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHER_INL
