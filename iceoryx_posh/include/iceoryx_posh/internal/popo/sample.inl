// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_POPO_SAMPLE_INL
#define IOX_POSH_POPO_SAMPLE_INL

#include "iceoryx_posh/popo/sample.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename H>
template <typename S, typename>
void Sample<T, H>::publish() noexcept
{
    if (BaseType::m_members.smartChunkUniquePtr)
    {
        BaseType::m_members.producerRef.get().publish(std::move(*(this)));
    }
    else
    {
        IOX_LOG(ERROR) << "Tried to publish empty Sample! Might be an already published or moved Sample!";
        errorHandler(PoshError::POSH__PUBLISHING_EMPTY_SAMPLE, ErrorLevel::MODERATE);
    }
}
} // namespace popo
} // namespace iox

#endif
