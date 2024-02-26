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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_ICEORYX_POSH_CONFIG_HPP
#define IOX_POSH_ICEORYX_POSH_CONFIG_HPP

#include "iceoryx_posh/mepoo/segment_config.hpp"
#include "iox/detail/deprecation_marker.hpp"
#include "roudi/roudi_config.hpp"

namespace iox
{
namespace helper
{
template <typename... ConfigParts>
struct SetDefaults;

template <typename FirstArg, typename... RemainderArgs>
struct SetDefaults<FirstArg, RemainderArgs...>
{
    template <typename BaseType>
    static void apply(BaseType* me) noexcept
    {
        SetDefaults<FirstArg>::apply(me);
        SetDefaults<RemainderArgs...>::apply(me);
    }
};

template <typename FinalArg>
struct SetDefaults<FinalArg>
{
    template <typename BaseType>
    static void apply(BaseType* me) noexcept
    {
        static_cast<FinalArg*>(me)->setDefaults();
    }
};

template <typename... ConfigParts>
struct Optimize;

template <typename FirstArg, typename... RemainderArgs>
struct Optimize<FirstArg, RemainderArgs...>
{
    template <typename BaseType>
    static void apply(BaseType* me) noexcept
    {
        Optimize<FirstArg>::apply(me);
        Optimize<RemainderArgs...>::apply(me);
    }
};

template <typename FinalArg>
struct Optimize<FinalArg>
{
    template <typename BaseType>
    static void apply(BaseType* me) noexcept
    {
        static_cast<FinalArg*>(me)->optimize();
    }
};
} // namespace helper

template <typename... ConfigParts>
struct Config : public ConfigParts...
{
    Config& setDefaults() noexcept
    {
        helper::SetDefaults<ConfigParts...>::apply(this);
        return *this;
    }

    template <typename T>
    Config& setModuleDefaults() noexcept
    {
        T::setDefaults();
        return *this;
    }

    Config& optimize() noexcept
    {
        helper::Optimize<ConfigParts...>::apply(this);
        return *this;
    }
};

using IceoryxConfig = Config<mepoo::SegmentConfig, config::RouDiConfig>;

using RouDiConfig_t IOX_DEPRECATED_SINCE(3, "Please use 'iox::IceoryxConfig' instead.") = IceoryxConfig;
} // namespace iox

#endif // IOX_POSH_ICEORYX_POSH_CONFIG_HPP
