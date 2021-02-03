# Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

import os

import unittest

import launch
from launch_ros.substitutions import ExecutableInPackage
import launch_testing
import launch_testing.actions
from launch_testing.asserts import assertSequentialStdout

import pytest


@pytest.mark.launch_test
def generate_test_description():

    proc_env = os.environ.copy()

    roudi_executable = ExecutableInPackage(
        package='iceoryx_systemtest', executable='iox-roudi')
    roudi_process = launch.actions.ExecuteProcess(
        cmd=[roudi_executable, '-l', 'debug'],
        env=proc_env, output='screen',
        sigterm_timeout='20'
    )

    publisher_executable = ExecutableInPackage(
        package='iceoryx_systemtest', executable='iox-publisher-systemtest')
    publisher_process = launch.actions.ExecuteProcess(
        cmd=[publisher_executable],
        env=proc_env, output='screen')

    subscriber_executable = ExecutableInPackage(
        package='iceoryx_systemtest', executable='iox-subscriber-systemtest')
    subscriber_process = launch.actions.ExecuteProcess(
        cmd=[subscriber_executable],
        env=proc_env, output='screen')

    return launch.LaunchDescription([
        roudi_process,
        publisher_process,
        subscriber_process,
        launch_testing.actions.ReadyToTest()
    ]), {'roudi_process': roudi_process, 'publisher_process': publisher_process, 'subscriber_process': subscriber_process}

#These tests will run concurrently with the dut process. After this test is done,
#the launch system will shut down RouDi
class TestProcess(unittest.TestCase):
    def test_roudi_ready(self, proc_output):
        proc_output.assertWaitFor(
            'RouDi is ready for clients', timeout=45, stream='stdout')

    def test_apps_ready(self, proc_output):
        proc_output.assertWaitFor(
            'Application iox_publisher_systemtest started', timeout=45, stream='stdout')
        proc_output.assertWaitFor(
            'Application iox_subscriber_systemtest started', timeout=45, stream='stdout')

    def test_simple_data_exchange(self, proc_output):
        proc_output.assertWaitFor(
            'Sent two times value: 5', timeout=45, stream='stdout')
        proc_output.assertWaitFor(
            'Got value: 5', timeout=45, stream='stdout')

# These tests run after shutdown and examine the stdout log
@launch_testing.post_shutdown_test()
class TestProcessOutput(unittest.TestCase):
    def test_exit_code(self, proc_info):
        launch_testing.asserts.assertExitCodes(proc_info)

    def test_publisher_sequence_output(self, proc_output, publisher_process):
        with assertSequentialStdout(proc_output, publisher_process) as cm:
            cm.assertInStdout('Sent two times value: 1')
            cm.assertInStdout('Sent two times value: 2')
            cm.assertInStdout('Sent two times value: 3')
            cm.assertInStdout('Sent two times value: 4')
            cm.assertInStdout('Sent two times value: 5')

    def test_subscriber_sequence_output(self, proc_output, subscriber_process):
        with assertSequentialStdout(proc_output, subscriber_process) as cm:
            #cm.assertInStdout('Got value: 1')
            cm.assertInStdout('Got value: 2')
            cm.assertInStdout('Got value: 3')
            cm.assertInStdout('Got value: 4')
            cm.assertInStdout('Got value: 5')
