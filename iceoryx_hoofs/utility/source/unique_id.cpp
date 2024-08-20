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

#include "iox/detail/unique_id.hpp"

namespace iox
{
// start with 1, just in case we want to use 0 for a special purpose later on
// NOLINTJUSTIFICATION see argument in header
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
concurrent::Atomic<uint64_t> UniqueId::m_IdCounter{1U};

UniqueId::UniqueId() noexcept
    : ThisType(newtype::internal::ProtectedConstructor, m_IdCounter.fetch_add(1U, std::memory_order_relaxed))
{
}
} // namespace iox
