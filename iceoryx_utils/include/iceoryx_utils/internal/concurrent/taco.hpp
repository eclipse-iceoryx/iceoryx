// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"

#include <atomic>
#include <cstdint>

namespace iox
{
namespace concurrent
{
/// Defines how the TACO works with data reads from the same context
enum class TACOMode
{
    /// Accept data reads from the same context as data write
    AccecptDataFromSameContext,
    /// Deny data reads from the same context as data write
    DenyDataFromSameContext
};

/// @brief
/// TACO is an acronym for Thread Aware exChange Ownership.
/// Exchanging data between thread needs some synchonization mechanism.
/// This can be done with a mutex or atomics. If the data structure is larger
/// than 64 bit or if more than one value need to be accessed in a synchronized
/// manner, a mutex would be the only option.
/// The TACO is a wait-free alternative to the mutex. Data can be exchanged
/// between threads. The TACO is like a SoFi with one element, but with the
/// possibility to read/write from multiple threads.
///
/// @param T       DataType to be stored
/// @param Context Enum class with all the thread context that access the TACO.
///                The enum must start with 0, must have ascending values and
///                the last vaule must be called END_OF_LIST.
///
/// @code
/// #include "iceoryx_utils/internal/concurrent/taco.hpp"
///
/// #include <cstdint>
/// #include <iostream>
/// #include <thread>
///
/// constexpr std::uint64_t TotalCount{1000000};
/// struct SyncedData
/// {
///     std::uint64_t decrementCounter{TotalCount};
///     std::uint64_t incrementCounter{0};
/// };
///
/// enum class ThreadContext : uint32_t
/// {
///     Hardy,
///     Laurel,
///     END_OF_LIST
/// };
///
/// int main()
/// {
///     concurrent::TACO<SyncedData, ThreadContext> taco(concurrent::TACOMode::DenyDataFromSameContext);
///     constexpr auto producerContext {ThreadContext::Hardy};
///     constexpr auto consumerContext {ThreadContext::Laurel};
///
///     auto producer = std::thread([&] {
///         SyncedData data;
///         while (data.decrementCounter != 0)
///         {
///             data.decrementCounter--;
///             data.incrementCounter++;
///             taco.store(data, producerContext);
///         }
///     });
///     auto consumer = std::thread([&] {
///         SyncedData data;
///         do
///         {
///             auto retVal = taco.take(consumerContext);
///             if (retVal.has_value())
///             {
///                 data = *retVal;
///                 if(data.decrementCounter + data.incrementCounter != TotalCount)
///                 {
///                     std::cout << "Error! Counter not synchronized!" << std::endl;
///                 }
///             }
///
///         } while (data.decrementCounter != 0);
///     });
///
///     producer.join();
///     consumer.join();
///
///     std::cout << "Finished!" << std::endl;
///
///     return 0;
/// }
/// @endcode
template <typename T, typename Context, uint32_t MaxNumberOfContext = 500>
class TACO
{
  private:
    struct Transaction
    {
        Context context{Context::END_OF_LIST};
        cxx::optional<T> data{cxx::nullopt_t()};
    };

    TACOMode m_mode;
    // this is the index of the transaction currently available for consumption
    std::atomic<uint32_t> m_pendingTransaction;

    static constexpr uint32_t NumberOfContext = static_cast<uint32_t>(Context::END_OF_LIST);
    // the corresponding transaction indices for the thread context;
    // the value of m_indicex[Context] contains the index of the m_transactions array which is owned by the context
    // so it's save to access m_transactions[m_indices[Context]]
    uint32_t m_indices[NumberOfContext];
    // this is a local buffer for the transaction, one for each thread that might access the TACO
    // and there needs to be one more element which is the one ready for consumption
    Transaction m_transactions[NumberOfContext + 1];

  public:
    /// Create a TACO instance with the specified mode
    /// @param [in] mode the TACO operates
    TACO(TACOMode mode)
        : m_mode(mode)
        , m_pendingTransaction(NumberOfContext)
    {
        static_assert(std::is_enum<Context>::value, "TACO Context must be an enum class!");
        static_assert(std::is_convertible<Context, uint32_t>::value == false,
                      "TACO Context must be an enum class, not just an enum!");
        static_assert(std::is_same<uint32_t, typename std::underlying_type<Context>::type>::value,
                      "TACO Context underlying type must be uint32_t!");
        static_assert(static_cast<uint32_t>(Context::END_OF_LIST) < MaxNumberOfContext,
                      "TACO exceeded max number of contexts!");

        // initially assign the indices to the corresponding contexts
        int i = 0;
        for (auto& index : m_indices)
        {
            index = i;
            i++;
        }
    }

    TACO(const TACO&) = delete;
    TACO(TACO&&) = delete;
    TACO& operator=(const TACO&) = delete;
    TACO& operator=(TACO&&) = delete;

    /// Takes the data from the TACO and supplies new data
    /// @param [in] data to supply for consumption, it's copied into a local cache in the TACO
    /// @param [in] context of the thread which performs the exchange
    /// @return the data a previous operation supplied for consumption or nullopt_t if there was either no data or the
    /// data was supplied from the same context and the mode disallows data from the same context
    cxx::optional<T> exchange(const T& data, Context context)
    {
        cxx::Expects(context < Context::END_OF_LIST);
        m_transactions[m_indices[static_cast<uint32_t>(context)]].data.emplace(data);
        return exchange(context);
    }

    /// Takes the data which is ready for consumption. The data isn't available for other access anymore.
    /// @param [in] context of the thread which takes the data
    /// @return the data a previous operation supplied for consumption or nullopt_t if there was either no data or the
    /// data was supplied from the same context and the mode disallows data from the same context
    cxx::optional<T> take(const Context context)
    {
        cxx::Expects(context < Context::END_OF_LIST);
        // there is no need to set the transaction for the corresponding context to nullopt_t, the exchange function
        // either moves the data, which leaves a nullopt_t or resets the data, which also results in a nullopt_t
        return exchange(context);
    }

    /// Supplies data for consumption
    /// @param [in] data to supply for consumption, it's copied into a local cache in the TACO
    /// @param [in] context of the thread which performs the exchange
    void store(const T& data, const Context context)
    {
        cxx::Expects(context < Context::END_OF_LIST);
        exchange(data, context);
    }

  private:
    cxx::optional<T> exchange(const Context context)
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
};
} // namespace concurrent
} // namespace iox

