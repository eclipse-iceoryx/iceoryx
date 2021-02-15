// Copyright (c) 2021 Apex.AI Inc. All rights reserved.
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

#include "iceoryx_utils/cxx/expected.hpp"

namespace iox
{
namespace popo
{
template <typename T, typename Status>
class Result : public iox::cxx::expected<T, Status>
{
  public:
    // todo: efficient construction without success/error
    // Result(const T& value)
    //     : iox::cxx::expected<T, Status>(iox::cxx::success<T>(value))
    // {
    // }


    // Result(Status&& value)
    //     : iox::cxx::expected<T, Status>(iox::cxx::error<Status>(value))
    // {
    // }

    Result(const iox::cxx::success<T>& value)
        : iox::cxx::expected<T, Status>(value)
    {
    }

    Result(iox::cxx::success<T>&& value)
        : iox::cxx::expected<T, Status>(value)
    {
    }


    Result(const iox::cxx::error<Status>& error)
        : iox::cxx::expected<T, Status>(error)
    {
    }
};

// todo: construct directly from T and Status and other helpers (if empty like)
} // namespace popo
} // namespace iox