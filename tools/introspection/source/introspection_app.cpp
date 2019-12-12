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

#include "iceoryx_introspection/introspection_app.hpp"
#include "iceoryx_introspection/introspection_run.hpp"
#include "iceoryx_introspection/introspection_types.hpp"
#include "iceoryx_utils/internal/units/duration.hpp"
#include "iceoryx_versions.hpp"

#include <iostream>

using namespace iox::client::introspection;
using namespace iox::units::duration_literals;

namespace iox
{
namespace client
{
namespace introspection
{
IntrospectionApp::IntrospectionApp(int argc, char* argv[]) noexcept
{
    if (argc < 2)
    {
        printShortInfo(argv[0]);
        exit(EXIT_FAILURE);
    }

    processArgs(argc, argv);
}

void IntrospectionApp::printHelp() noexcept
{
    std::cout << "Usage:\n"
                 "  introspection [OPTIONS] [SUBSCRIPTION]\n"
                 "  introspection --help\n"
                 "  introspection --version\n"
                 "\nOptions:\n"
                 "  -h, --help        Display help and exit.\n"
                 "  -t, --time <ms>   Update period (in milliseconds) for the display of introspection data\n"
                 "                    [min: "
              << MIN_UPDATE_PERIOD.milliSeconds<int>()
              << ", max: " << MAX_UPDATE_PERIOD.milliSeconds<int>()
              << ", default: " << DEFAULT_UPDATE_PERIOD.milliSeconds<int>()
              << "]\n"
                 "  -v, --version     Display latest official iceoryx release version and exit.\n"
                 "\nSubscription:\n"
                 "  Select which introspection data you would like to receive.\n"
                 "  --all             Subscribe to all available introspection data.\n"
                 "  --mempool         Subscribe to mempool introspection data.\n"
                 "  --port            Subscribe to port introspection data.\n"
                 "  --process         Subscribe to process introspection data.\n"
              << std::endl;
}

void IntrospectionApp::printShortInfo(const std::string& binaryName) noexcept
{
    std::cout << "Run '" << binaryName << " --help' for more information." << std::endl;
}

void IntrospectionApp::processArgs(int argc, char** argv) noexcept
{
    int opt;
    int index;

    while (
        (opt = getopt_long(argc, argv, shortOptions, longOptions, &index))
        != -1)
    {
        switch (opt)
        {
        case 'h':
            printHelp();
            exit(EXIT_SUCCESS);
            break;

        case 'v':
            std::cout << "Latest official IceOryx release version: " << ICEORYX_LATEST_RELEASE_VERSION << "\n"
                      << std::endl;
            exit(EXIT_SUCCESS);
            break;

        case 't':
        {
            int l_rate = std::atoi(optarg);
            updatePeriodMs = bounded(l_rate,
                                     MIN_UPDATE_PERIOD.milliSeconds<int>(),
                                     MAX_UPDATE_PERIOD.milliSeconds<int>());
            break;
        }

        case 0:
            if (longOptions[index].flag != 0)
                break;

            if (strcmp(longOptions[index].name, "all") == 0)
            {
                introspectionSelection.mempool = introspectionSelection.port = introspectionSelection.process = true;
                doIntrospection = true;
            }
            else if (strcmp(longOptions[index].name, "port") == 0)
            {
                introspectionSelection.port = true;
                doIntrospection = true;
            }
            else if (strcmp(longOptions[index].name, "process") == 0)
            {
                introspectionSelection.process = true;
                doIntrospection = true;
            }
            else if (strcmp(longOptions[index].name, "mempool") == 0)
            {
                introspectionSelection.mempool = true;
                doIntrospection = true;
            }

            break;

        case '?':
        default:
            printShortInfo(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (!doIntrospection)
    {
        std::cout << "Wrong usage. ";
        printShortInfo(argv[0]);
        exit(EXIT_FAILURE);
    }
}

} // namespace introspection
} // namespace client
} // namespace iox
