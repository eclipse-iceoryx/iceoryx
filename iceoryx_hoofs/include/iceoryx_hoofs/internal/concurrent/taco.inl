// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_HOOFS_CONCURRENT_TACO_INL
#define IOX_HOOFS_CONCURRENT_TACO_INL

#include "iceoryx_hoofs/internal/concurrent/taco.hpp"

namespace iox
{
namespace concurrent
{
template <typename T, typename Context, uint32_t MaxNumberOfContext>
inline TACO<T, Context, MaxNumberOfContext>::TACO(TACOMode mode) noexcept
    : m_mode(mode)
    , m_pendingTransaction(NumberOfContext)
{
    static_assert(std::is_enum<Context>::value, "TACO Context must be an enum class!");
    static_assert(!std::is_convertible<Context, uint32_t>::value,
                  "TACO Context must be an enum class, not just an enum!");
    static_assert(std::is_same<uint32_t, typename std::underlying_type<Context>::type>::value,
                  "TACO Context underlying type must be uint32_t!");
    static_assert(static_cast<uint32_t>(Context::END_OF_LIST) < MaxNumberOfContext,
                  "TACO exceeded max number of contexts!");

    // initially assign the indices to the corresponding contexts
    uint32_t i = 0;
    for (auto& index : m_indices)
    {
        index = i;
        i++;
    }
}

template <typename T, typename Context, uint32_t MaxNumberOfContext>
inline cxx::optional<T> TACO<T, Context, MaxNumberOfContext>::exchange(const T& data, Context context) noexcept
{
    cxx::Expects(context < Context::END_OF_LIST);
    m_transactions[m_indices[static_cast<uint32_t>(context)]].data.emplace(data);
    return exchange(context);
}

template <typename T, typename Context, uint32_t MaxNumberOfContext>
inline cxx::optional<T> TACO<T, Context, MaxNumberOfContext>::take(const Context context) noexcept
{
    cxx::Expects(context < Context::END_OF_LIST);
    // there is no need to set the transaction for the corresponding context to nullopt_t, the exchange function
    // either moves the data, which leaves a nullopt_t or resets the data, which also results in a nullopt_t
    return exchange(context);
}

template <typename T, typename Context, uint32_t MaxNumberOfContext>
inline void TACO<T, Context, MaxNumberOfContext>::store(const T& data, const Context context) noexcept
{
    cxx::Expects(context < Context::END_OF_LIST);
    exchange(data, context);
}

template <typename T, typename Context, uint32_t MaxNumberOfContext>
inline cxx::optional<T> TACO<T, Context, MaxNumberOfContext>::exchange(const Context context) noexcept
{
    auto contextIndex = static_cast<uint32_t>(context);
    auto transactionIndexOld = m_indices[contextIndex];
    m_transactions[transactionIndexOld].context = context;

    m_indices[contextIndex] = m_pendingTransaction.exchange(transactionIndexOld, std::memory_order_acq_rel);
    auto transactionIndexNew = m_indices[contextIndex];

    if (m_mode == TACOMode::AccecptDataFromSameContext || m_transactions[transactionIndexNew].context != context)
    {
        return std::move(m_transactions[transactionIndexNew].data);
    }

    m_transactions[transactionIndexNew].data.reset();
    return cxx::nullopt_t();
}
} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_TACO_INL
