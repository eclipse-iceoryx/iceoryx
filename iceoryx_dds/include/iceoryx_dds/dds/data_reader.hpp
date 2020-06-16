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

#ifndef IOX_DDS_DDS_DATA_READER_HPP
#define IOX_DDS_DDS_DATA_READER_HPP

#include <iceoryx_utils/cxx/string.hpp>

namespace iox
{
namespace dds
{
using IdString = iox::cxx::string<100u>;

template <typename Impl>
class DataReader
{
};
}
}

#endif // IOX_DDS_DDS_DATA_READER_HPP
