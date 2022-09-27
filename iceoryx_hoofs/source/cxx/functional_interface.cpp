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

#include "iceoryx_hoofs/cxx/functional_interface.hpp"
#include "iceoryx_hoofs/log/logging.hpp"

namespace iox
{
namespace cxx
{
namespace internal
{
void print_expect_message(const char* message) noexcept
{
    // print_expect_message is only called from expect. expect allows only
    // cxx::string or char arrays which are both correctly null terminated
    IOX_LOG(FATAL) << message;
}
} // namespace internal
} // namespace cxx
} // namespace iox
