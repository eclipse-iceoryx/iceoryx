#!/bin/bash

# cd build/binding_c/test
# ./binding_c_integrationtests --gtest_filter="*iox_ws_test*" 2>&1 | tee log_file_ignore.txt
# code log_file_ignore.txt
# cd ../../..

cd build/utils/test
./utils_moduletests --gtest_filter="*IpcChannel_test*" 2>&1  | tee log_func_ignore.txt
code log_func_ignore.txt
cd ../../..
