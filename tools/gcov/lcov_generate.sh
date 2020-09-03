#!/bin/bash

# Copyright (c) 2019-2020 by Robert Bosch GmbH. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

BUILD_FOLDER=${1:-$PWD}
COMMAND=${2:-$capture} 
OUTPUT_FOLDER=$BUILD_FOLDER/build/lcov

mkdir -p $OUTPUT_FOLDER

case "$2" in    
    "initial")
        lcov -c -i -d $BUILD_FOLDER -o $OUTPUT_FOLDER/iceoryx_init.info --no-external --rc lcov_branch_coverage=1
        ;;
    "capture")    
        lcov -c -d $BUILD_FOLDER -o $OUTPUT_FOLDER/iceoryx_test.info --no-external --rc lcov_branch_coverage=1
        ;;
    "combine") 
        lcov -a $OUTPUT_FOLDER/iceoryx_init.info --add-tracefile $OUTPUT_FOLDER/iceoryx_test.info -o $OUTPUT_FOLDER/iceoryx_full.info --rc lcov_branch_coverage=1
        ;;
    "remove") 
        lcov -o $OUTPUT_FOLDER/iceoryx_filter.info --rc lcov_branch_coverage=1 -r $OUTPUT_FOLDER/iceoryx_full.info "*/build/*" "*/test/*" "*/testutils/*" "*/roudi_environment/*"
        ;;
    "genhtml") 
        genhtml $OUTPUT_FOLDER/iceoryx_filter.info -o $OUTPUT_FOLDER --config-file $BUILD_FOLDER/tools/gcov/lcovr_html.conf --legend --show-details --branch-coverage
        ;;
esac
