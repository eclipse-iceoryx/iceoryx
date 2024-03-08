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
$NumCPUs = (Get-WmiObject Win32_processor).NumberOfLogicalProcessors

switch ($toolchain) {
    "MSVC" {
        if ($?) { Write-Host "## Building sources with MSVC toolchain" }
        # We require the Windows SDK Version 10.0.18362.0 since a previous version had a bug which caused a fatal compilation error within iceoryx and was
        # fixed with this version, see: https://github.com/microsoft/vcpkg/issues/15035#issuecomment-742427969.
        if ($?) { cmake -Bbuild -Hiceoryx_meta -DBUILD_TEST=ON -DINTROSPECTION=OFF -DBINDING_C=ON -DEXAMPLES=ON -DCMAKE_CXX_FLAGS="/MP" -DCMAKE_SYSTEM_VERSION="10.0.18362.0" }
    }
    "MinGW" {
        if ($?) { Write-Host "## Building sources with MinGW toolchain" }
        if ($?) { cmake -Bbuild -Hiceoryx_meta -DBUILD_TEST=ON -DINTROSPECTION=OFF -DBINDING_C=ON -DEXAMPLES=ON -DCMAKE_SYSTEM_VERSION="10.0.18362.0" -G "MinGW Makefiles" }
    }
    default {
        if ($?) { Write-Host "## The '$toolchain' toolchain is not supported. Currently only 'MSVC' and 'MingGW' is supported to build iceoryx." }
        exit 1
    }
}

if ($?) { Write-Host "## Building with $NumCPUs cores" }
if ($?) { cmake --build build -j $NumCPUs }

switch ($toolchain ) {
    "MSVC" {
        if ($?) { Write-Host "## Copy test binaries to common location" }
        if ($?) { Copy-Item -Force build\platform\test\Debug\*.exe build\platform\test\ }
        if ($?) { Copy-Item -Force build\hoofs\test\Debug\*.exe build\hoofs\test\ }
        if ($?) { Copy-Item -Force build\posh\test\Debug\*.exe build\posh\test\ }
        if ($?) { Copy-Item -Force build\binding_c\test\Debug\*.exe build\binding_c\test\ }
    }
    default {
        # nothing to do
    }
}

if ($?) { Write-Host "## Running tests (excluding timing_tests)" }
# until the windows support is fully implemented and we can use the windows cmake targets
# we have to exclude the tests explicitly until everyone is running

if ($?) { build\platform\test\platform_moduletests.exe }
if ($?) { build\platform\test\platform_integrationtests.exe }
if ($?) { build\hoofs\test\hoofs_mocktests.exe }
if ($?) { build\hoofs\test\hoofs_moduletests.exe --gtest_filter="-*TimingTest*:AdaptiveWait*" }
if ($?) { build\hoofs\test\hoofs_integrationtests.exe }
if ($?) { build\posh\test\posh_moduletests.exe --gtest_filter="-ChunkHeader_test.ChunkHeaderBinaryCompatibilityCheck:TomlGatewayConfigParser*:IceoryxRoudiApp_test.ConstructorCalledWithArgUniqueIdTwoTimesReturnError:IceoryxRoudiApp_test.ConstructorCalledWithArgVersionSetRunVariableToFalse:ValidTest*:ParseAllMalformedInput*:*TimingTest*:MePooSegment_test.SharedMemoryFileHandleRightsAfterConstructor:PoshRuntimeSingleProcess_test*" }
# The PoshRuntimeSingleProcess_test brings the system into a state from which it will not recover until the process is terminated
if ($?) { build\posh\test\posh_moduletests.exe --gtest_filter="PoshRuntimeSingleProcess_test*" }
if ($?) { build\posh\test\posh_integrationtests.exe --gtest_filter="-ChunkBuildingBlocks_IntegrationTest.TwoHopsThreeThreadsNoSoFi:*TimingTest*" }
if ($?) { build\binding_c\test\binding_c_moduletests.exe --gtest_filter="-BindingC_Runtime_test.RuntimeNameLengthIsOutOfLimit:BindingC_Runtime_test.RuntimeNameIsNullptr:*TimingTest*" }
if ($?) { build\binding_c\test\binding_c_integrationtests.exe }

exit $LASTEXITCODE
