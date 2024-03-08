// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI inc. All rights reserved.
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

#include "iceoryx_hoofs/testing/error_reporting/testing_error_handler.hpp"
#include "iceoryx_hoofs/testing/testing_logger.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include "test.hpp"

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    iox::experimental::hasExperimentalPoshFeaturesEnabled(true);

    iox::testing::TestingLogger::init();
    iox::testing::TestingErrorHandler::init();

    return RUN_ALL_TESTS();
}
