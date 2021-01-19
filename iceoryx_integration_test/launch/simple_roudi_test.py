# -*- coding: utf-8 -*-

# Copyright 2020 Apex.AI, Inc.
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
import sys
import unittest
import time
import subprocess

import ament_index_python

import launch
import launch.actions
from launch_ros.substitutions import ExecutableInPackage
from launch import LaunchDescription

import launch_testing
import launch_testing.actions
from launch_testing.asserts import assertSequentialStdout

import pytest


@pytest.mark.launch_test
def generate_test_description():

    proc_env = os.environ.copy()

    roudi_executable = ExecutableInPackage(
        package='iceoryx_integration_test', executable='iox-roudi')

    dut_process = launch.actions.ExecuteProcess(
        cmd=[roudi_executable, '-l', 'debug'],
        env=proc_env, output='screen'
    )

    return launch.LaunchDescription([
        dut_process,
        launch_testing.actions.ReadyToTest()
    ]), {'dut_process': dut_process}


# These tests will run concurrently with the dut process. After this test is done,
# the launch system will shut down RouDi
class TestRouDiProcess(unittest.TestCase):
    def test_roudi_ready(self, proc_output):
        proc_output.assertWaitFor(
            'RouDi is ready for clients', timeout=45, stream='stdout')


@launch_testing.post_shutdown_test()
class TestProcessOutput(unittest.TestCase):

    def test_exit_code(self, proc_info):
        launch_testing.asserts.assertExitCodes(proc_info)

    def test_full_output(self, proc_output, dut_process):
        # Using the SequentialStdout context manager asserts that the following stdout
        # happened in the same order that it's checked
        with assertSequentialStdout(proc_output, dut_process) as cm:
            cm.assertInStdout('Log level set to:')
            cm.assertInStdout('[ Reserving shared memory successful ]')
            cm.assertInStdout('Registered memory segment')
            cm.assertInStdout('[ Reserving shared memory successful ]')
            cm.assertInStdout('Roudi registered payload segment')
            cm.assertInStdout('RouDi is ready for clients')

            if os.name != 'nt':
                # On Windows, process termination is always forced
                # and thus the last print in good_proc never makes it.
                cm.assertInStdout("Joining 'ProcessMgmt' thread...")
                cm.assertInStdout("...'ProcessMgmt' thread joined.")
                cm.assertInStdout("Joining 'MQ-processing' thread...")
                cm.assertInStdout("...'MQ-processing' thread joined.")

    def test_out_of_order(self, proc_output, dut_process):
        # notice out-of-order IO
        with assertSequentialStdout(proc_output, dut_process) as cm:
            cm.assertInStdout('Log level set to:')
            cm.assertInStdout('[ Reserving shared memory successful ]')
            cm.assertInStdout('Registered memory segment')
            cm.assertInStdout('[ Reserving shared memory successful ]')
            cm.assertInStdout('Roudi registered payload segment')
            cm.assertInStdout('RouDi is ready for clients')
