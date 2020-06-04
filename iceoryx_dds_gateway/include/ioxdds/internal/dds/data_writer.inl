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

template <typename Impl>
inline void iox::dds::DataWriter<Impl>::connect() noexcept
{
    return static_cast<Impl*>(this)->connect();
};

template <typename Impl>
inline void iox::dds::DataWriter<Impl>::write(const uint8_t* bytes, const uint64_t size) noexcept
{
    return static_cast<Impl*>(this)->write(bytes, size);
};

template <typename Impl>
inline iox::dds::IdString iox::dds::DataWriter<Impl>::getServiceId() const noexcept
{
    return static_cast<Impl*>(this)->getServiceId();
};

template <typename Impl>
inline iox::dds::IdString iox::dds::DataWriter<Impl>::getInstanceId() const noexcept
{
    return static_cast<Impl*>(this)->getInstanceId();
};

template <typename Impl>
inline iox::dds::IdString iox::dds::DataWriter<Impl>::getEventId() const noexcept
{
    return static_cast<Impl*>(this)->getEventId();
};
