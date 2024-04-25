// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_hoofs/testing/mocks/logger_mock.hpp"
#include "iceoryx_hoofs/testing/test.hpp"
#include "iox/log/logstream.hpp"
#include "iox/logging.hpp"

using namespace ::testing;

using iox::testing::Logger_Mock;

class LogStreamSut : public iox::log::LogStream
{
  public:
    explicit LogStreamSut(iox::log::Logger& logger)
        : iox::log::LogStream(logger, "file", 42, "function", iox::log::LogLevel::TRACE)
    {
    }
};

class IoxLogStreamBase_test : public Test
{
  public:
    Logger_Mock loggerMock;
};
