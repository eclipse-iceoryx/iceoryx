# Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#
# SPDX-License-Identifier: Apache-2.0

# This script builds iceoryx_hoofs und iceoryx_posh and executes all tests

param(
    [Parameter()]
    [String]$toolchain = "MSVC"
)

$ErrorActionPreference = "Stop"

switch ($toolchain) {
    "MSVC" {
        if ($?) { Write-Host "Building sources with MSVC toolchain" }
        # We require the Windows SDK Version 10.0.18362.0 since a previous version had a bug which caused a fatal compilation error within iceoryx and was
        # fixed with this version, see: https://github.com/microsoft/vcpkg/issues/15035#issuecomment-742427969.
        if ($?) { cmake -Bbuild -Hiceoryx_meta -DBUILD_TEST=ON -DINTROSPECTION=OFF -DBINDING_C=ON -DEXAMPLES=ON -DCMAKE_CXX_FLAGS="/MP" -DCMAKE_SYSTEM_VERSION="10.0.18362.0" }
    }
    "MinGW" {
        if ($?) { Write-Host "Building sources with MinGW toolchain" }
        if ($?) { cmake -Bbuild -Hiceoryx_meta -DBUILD_TEST=ON -DINTROSPECTION=OFF -DBINDING_C=ON -DEXAMPLES=ON -DCMAKE_SYSTEM_VERSION="10.0.18362.0" -G "MinGW Makefiles" }
    }
    default {
        if ($?) { Write-Host "The '$toolchain' toolchain is not supported. Currently only 'MSVC' and 'MingGW' is supported to build iceoryx." }
        exit 1
    }
}

if ($?) { cmake --build build }

if ($?) { Write-Host "running tests (excluding timing_tests)" }
# until the windows support is fully implemented and we can use the windows cmake targets
# we have to exclude the tests explicitly until everyone is running

if ($?) { build\hoofs\test\Debug\hoofs_moduletests.exe --gtest_filter="-*TimingTest*" }
if ($?) { build\hoofs\test\Debug\hoofs_mocktests.exe }
if ($?) { build\hoofs\test\Debug\hoofs_integrationtests.exe }
if ($?) { build\binding_c\test\Debug\binding_c_moduletests.exe --gtest_filter="-BindingC_Runtime_test.RuntimeNameLengthIsOutOfLimit:BindingC_Runtime_test.RuntimeNameIsNullptr:*TimingTest*" }
if ($?) { build\posh\test\Debug\posh_moduletests.exe --gtest_filter="-ChunkHeader_test.ChunkHeaderBinaryCompatibilityCheck:TomlGatewayConfigParser*:IceoryxRoudiApp_test.ConstructorCalledWithArgUniqueIdTwoTimesReturnError:IceoryxRoudiApp_test.ConstructorCalledWithArgVersionSetRunVariableToFalse:ValidTest*:ParseAllMalformedInput*:*TimingTest*:MePooSegment_test.SharedMemoryFileHandleRightsAfterConstructor" }
if ($?) { build\posh\test\Debug\posh_integrationtests.exe --gtest_filter="-ChunkBuildingBlocks_IntegrationTest.TwoHopsThreeThreadsNoSoFi:*TimingTest*" }

exit $LASTEXITCODE
