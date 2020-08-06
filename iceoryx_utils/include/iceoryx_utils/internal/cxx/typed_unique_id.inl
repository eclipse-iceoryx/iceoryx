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
#ifndef IOX_UTILS_CXX_TYPED_UNIQUE_ID_INL
#define IOX_UTILS_CXX_TYPED_UNIQUE_ID_INL

namespace iox
{
namespace cxx
{
template <typename T>
std::atomic<uint64_t> TypedUniqueId<T>::globalIDCounter{0u};

template <typename T>
inline TypedUniqueId<T>::TypedUniqueId() noexcept
    : ThisType(newtype::internal::ProtectedConstructor, globalIDCounter.fetch_add(1u, std::memory_order_relaxed))
{
}
} // namespace cxx
} // namespace iox
#endif
