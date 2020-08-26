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
#ifndef IOX_UTILS_CXX_HELPLETS_HPP
#define IOX_UTILS_CXX_HELPLETS_HPP

#include "iceoryx_utils/cxx/generic_raii.hpp"
#include "iceoryx_utils/platform/platform_correction.hpp"

#include <assert.h>
#include <iostream>
#include <type_traits>

namespace iox
{
namespace cxx
{
namespace internal
{
inline void
Require(const bool condition, const char* file, const int line, const char* function, const char* conditionString)
{
    if (!condition)
    {
        std::cerr << "Condition: " << conditionString << " in " << function << " is violated. (" << file << ":" << line
                  << ")" << std::endl;
        std::terminate();
    }
}
} // namespace internal

// implementing C++ Core Guideline, I.6. Prefer Expects
// see:
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-expects
#define Expects(condition) internal::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition)

// implementing C++ Core Guideline, I.8. Prefer Ensures
// see:
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-ensures
#define Ensures(condition) internal::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition)

template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value, void>::type>
struct not_null
{
  public:
    not_null(T t)
        : value(t)
    {
        Expects(t != nullptr);
    }
    constexpr operator T() const
    {
        return value;
    }

  private:
    T value;
};

template <typename T, T Minimum>
struct greater_or_equal
{
  public:
    greater_or_equal(T t)
        : value(t)
    {
        Expects(t >= Minimum);
    }

    constexpr operator T() const
    {
        return value;
    }

  private:
    T value;
};

template <typename T, T Minimum, T Maximum>
struct range
{
  public:
    range(T t)
        : value(t)
    {
        Expects(t >= Minimum && t <= Maximum);
    }

    constexpr operator T() const
    {
        return value;
    }

  private:
    T value;
};

template <typename T>
T align(const T value, const T alignment)
{
    T remainder = value % alignment;
    return value + ((remainder == 0u) ? 0u : alignment - remainder);
}

/// @brief allocates aligned memory which can only be free'd by alignedFree
/// @param[in] alignment, alignment of the memory
/// @param[in] size, memory size
/// @return void pointer to the aligned memory
void* alignedAlloc(const uint64_t alignment, const uint64_t size) noexcept;

/// @brief frees aligned memory allocated with alignedAlloc
/// @param[in] memory, pointer to the aligned memory
void alignedFree(void* const memory);

/// template recursion stopper for maximum alignment calculation
template <size_t s = 0>
constexpr size_t maxAlignment()
{
    return s;
}

/// calculate maximum alignment of supplied types
template <typename T, typename... Args>
constexpr size_t maxAlignment()
{
    return alignof(T) > maxAlignment<Args...>() ? alignof(T) : maxAlignment<Args...>();
}

/// template recursion stopper for maximum size calculation
template <size_t s = 0>
constexpr size_t maxSize()
{
    return s;
}

/// calculate maximum size of supplied types
template <typename T, typename... Args>
constexpr size_t maxSize()
{
    return sizeof(T) > maxSize<Args...>() ? sizeof(T) : maxSize<Args...>();
}

/// @todo better name
/// create a GenericRAII object to cleanup a static optional object at the end of the scope
/// @param [in] T memory container which has emplace(...) and reset
/// @param [in] CTorArgs ctor types for the object to construct
/// @param [in] memory is a reference to a memory container, e.g. cxx::optional
/// @param [in] ctorArgs ctor arguments for the object to construct
/// @return cxx::GenericRAII
template <typename T, typename... CTorArgs>
GenericRAII makeScopedStatic(T& memory, CTorArgs&&... ctorArgs)
{
    memory.emplace(std::forward<CTorArgs>(ctorArgs)...);
    return GenericRAII([] {}, [&memory] { memory.reset(); });
}
/// Convert Enum class type to string
template <typename T, typename Enumeration>
const char* convertEnumToString(T port, const Enumeration source)
{
    return port[static_cast<size_t>(source)];
}

/// cast an enum to its underlying type
template <typename enum_type>
auto enumTypeAsUnderlyingType(enum_type const value) -> typename std::underlying_type<enum_type>::type
{
    return static_cast<typename std::underlying_type<enum_type>::type>(value);
}

/// @brief Get the size of a string represented by a char array at compile time.
/// @tparam The size of the char array filled out by the compiler.
/// @param[in] The actual content of the char array is not of interest. Its just the size of the array that matters.
/// @return Returns the size of a char array at compile time.
template <uint64_t SizeValue>
static constexpr uint64_t strlen2(char const (&/*notInterested*/)[SizeValue])
{
    return SizeValue - 1;
}

} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_HELPLETS_HPP
