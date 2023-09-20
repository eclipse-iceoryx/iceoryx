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

import os

import unittest

import launch
from launch_ros.substitutions import ExecutableInPackage
import launch_testing
import launch_testing.actions
from launch_testing.asserts import assertSequentialStdout

import pytest

# @brief Test goal: "Integrationtest for the request response listener and waitset example of iceoryx"
# @pre setup ROS2 launch executables for RouDi (debug mode) and the example processes
# @post check if all applications return exitcode 0 (success) after test run
@pytest.mark.launch_test
def generate_test_description():

    proc_env = os.environ.copy()
    colcon_prefix_path = os.environ.get('COLCON_PREFIX_PATH', '')
    executable_list = ['iox-c-request-response-server-listener', 'iox-c-request-response-client-waitset']
    process_list = []

    for exec in executable_list:
        tmp_exec = os.path.join(
            colcon_prefix_path,
            'example_request_response_in_c/bin/',
            exec)
        tmp_process = launch.actions.ExecuteProcess(
            cmd=[tmp_exec],
            env=proc_env, output='screen')
        process_list.append(tmp_process)

    print("Process list:", process_list)

    roudi_executable = os.path.join(
        colcon_prefix_path,
        'iceoryx_posh/bin/',
        'iox-roudi'
    )
    roudi_process = launch.actions.ExecuteProcess(
        cmd=[roudi_executable, '-l', 'trace', '--termination-delay', '5', '--kill-delay', '5'],
        env=proc_env, output='screen',
        sigterm_timeout='20')

    return launch.LaunchDescription([
        process_list[0],
        process_list[1],
        roudi_process,
        launch_testing.actions.ReadyToTest()
    ]), {'iox-c-request-response-server-listener': process_list[0], 'iox-c-request-response-client-waitset': process_list[1],
         'roudi_process': roudi_process}

# These tests will run concurrently with the dut process. After this test is done,
# the launch system will shut down RouDi


class TestRequestResponseInCListenerAndWaitsetExample(unittest.TestCase):
    def test_roudi_ready(self, proc_output):
        proc_output.assertWaitFor(
            'RouDi is ready for clients', timeout=45, stream='stdout')

    def test_listener_server_client_data_exchange(self, proc_output):
        proc_output.assertWaitFor(
            'iox-c-request-response-client-waitset Send Request: 55 + 89', timeout=45, stream='stdout')
        proc_output.assertWaitFor(
            'iox-c-request-response-server-listener Got Request: 55 + 89', timeout=45, stream='stdout')

        proc_output.assertWaitFor(
            'iox-c-request-response-server-listener Send Response: 144', timeout=45, stream='stdout')
        proc_output.assertWaitFor(
            'iox-c-request-response-client-waitset Got Response: 144', timeout=45, stream='stdout')

# These tests run after shutdown and examine the stdout log
@launch_testing.post_shutdown_test()
class TestRequestResponseListenerExampleExitCodes(unittest.TestCase):
    def test_exit_code(self, proc_info):
        launch_testing.asserts.assertExitCodes(proc_info)
