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

#ifndef IOX_UTILS_CXX_TYPE_TRAITS_HPP
#define IOX_UTILS_CXX_TYPE_TRAITS_HPP

#include <type_traits>

namespace iox {
namespace cxx {

///
/// @brief Verifies whether the passed Callable type is in fact callable
///
template<typename Callable, typename... ArgTypes>
struct is_callable {

    // This variant is chosen when Callable(ArgTypes) successfully resolves to a valid type, i.e. is callable.
    template<typename C, typename... As>
    static constexpr std::true_type test(decltype(std::declval<C>()(std::declval<As>()...))*)
    {
        return {};
    }

    // This is chosen if Callable(ArgTypes) does not resolve to a valid type.
    template<typename C, typename... As>
    static constexpr std::false_type test(...)
    {
        return {};
    }

    // Test with nullptr as this can stand in for a pointer to any type.
    static constexpr bool value = decltype(test<Callable, ArgTypes...>(nullptr))::value;
};

///
/// @brief Verfies the signature of the provided callable type
///
template <typename Callable = void, typename ReturnType = void, typename ArgTypes = void>
struct has_signature : std::false_type {};

template <typename Callable, typename ReturnType, typename... ArgTypes>
struct has_signature<Callable, ReturnType(ArgTypes...),
    typename std::enable_if<
        std::is_convertible<
            decltype(std::declval<Callable>()(std::declval<ArgTypes>()...)),
            ReturnType
        >::value, void>::type>
    : std::true_type {};

} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_TYPE_TRAITS_HPP
