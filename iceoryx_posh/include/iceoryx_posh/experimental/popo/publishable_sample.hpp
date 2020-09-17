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

#ifndef IOX_EXPERIMENTAL_POSH_POPO_PUBLISHABLE_SAMPLE_HPP
#define IOX_EXPERIMENTAL_POSH_POPO_PUBLISHABLE_SAMPLE_HPP

#include "iceoryx_posh/experimental/popo/sample.hpp"

namespace iox {
namespace popo {

template<typename T>
class PublisherInterface;

template <typename T>
class PublishableSample : public Sample<T>
{
public:
    PublishableSample(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher);
    PublishableSample(std::nullptr_t) noexcept;
    PublishableSample(const PublishableSample&) = delete;
    PublishableSample& operator=(const PublishableSample&) = delete;
    PublishableSample(PublishableSample<T>&& rhs);
    PublishableSample& operator=(PublishableSample<T>&& rhs);

    PublishableSample& operator=(std::nullptr_t) noexcept;

    T* get() noexcept;
    void publish() noexcept;

private:
    bool m_hasOwnership{true};
    std::reference_wrapper<PublisherInterface<T>> m_publisherRef;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/experimental/internal/popo/publishable_sample.inl"

#endif // IOX_EXPERIMENTAL_POSH_POPO_PUBLISHABLE_SAMPLE_HPP

