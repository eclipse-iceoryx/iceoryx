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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_TYPED_UNIQUE_ID_INL
#define IOX_POSH_POPO_BUILDING_BLOCKS_TYPED_UNIQUE_ID_INL

namespace iox
{
namespace popo
{
template <typename T, uint16_t MajorOffset>
std::atomic<uint64_t> TypedUniqueId<T, MajorOffset>::globalIDCounter{0u};

template <typename T, uint16_t MajorOffset>
inline TypedUniqueId<T, MajorOffset>::Id::Id(const uint64_t minor) noexcept
    : m_major(MajorOffset)
    , m_minor(minor)
{
    uint64_t minorMaximum = static_cast<uint64_t>(1u) << 48;
    if (minor > minorMaximum)
    {
        std::cerr << "typed unique id overflow " << minor << " > " << minorMaximum << std::endl;
    }
}

template <typename T, uint16_t MajorOffset>
inline TypedUniqueId<T, MajorOffset>::Id::operator uint64_t() const noexcept
{
    return *reinterpret_cast<const uint64_t*>(this);
}

template <typename T, uint16_t MajorOffset>
inline TypedUniqueId<T, MajorOffset>::TypedUniqueId() noexcept
    : ThisType(cxx::newtype::internal::ProtectedConstructor,
               static_cast<uint64_t>(Id(globalIDCounter.fetch_add(1u, std::memory_order_relaxed))))
{
}
} // namespace popo
} // namespace iox
#endif
