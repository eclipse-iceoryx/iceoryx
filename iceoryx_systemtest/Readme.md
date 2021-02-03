# Iceoryx Systemtests

## Introduction
To ensure quality standards in iceoryx, we are using automated testing to verify functionality on unittest and integrationtest level.
Additionally we need to make sure that the customer-facing API is functional and system-level requirements are fulfilled.

For that purpose we bring in tests which simulate customer behavior to have automatic testing of (Mis)Use-cases.
As testing framework we use the [launch_testing](https://github.com/ros2/launch/tree/master/launch_testing) from ROS 2.
The tests are in a ROS 2 CMake package where the test executables are build and tested by python scripts.
In these scripts the testing is currently done by evaluating stdout output of the processes and the exit codes.

advantages:
- automatic test if processes have exited unexpectedly
- test against custom or error return codes to evaluate error-cases
- test the order of printed messages

limitations:
- testing against stdout is error prone
- limited functionality for performance testing because the stdout is buffered (messages could be reordered)

## Setup
For building and executing the tests you need to have ROS2 installed. Please follow the instructions on https://index.ros.org/doc/ros2/Installation.
The systemtests are currently tested on ROS 2 "Foxy Fitzroy" in Ubuntu 20.04 LTS.

For a basic setup you need to install the following packages:
```bash
sudo apt install ros-foxy-ros-base ros-foxy-ros-testing ros-foxy-launch-testing ros-foxy-ament-cmake python3-colcon-common-extensions
```

After installing you need to source ROS 2 to make the environment available in your terminal:
```bash
source /opt/ros/foxy/setup.bash
```

**_NOTE:_** You can add the source command to your `~/.bashrc` for automatic loading the ROS2 workspace at boot time.

Required for the colcon build of iceoryx is that the repository is located within a ROS workspace like this:
```
iceoryx_workspace
└── src
    └── iceoryx
        ├── cmake
        ├── cpptoml_vendor
        ├── doc
        ├── iceoryx_binding_c
        ├── iceoryx_dds
        ├── iceoryx_examples
        ├── iceoryx_meta
        ├── iceoryx_posh
        ├── iceoryx_systemtest
        ├── iceoryx_utils
        └── tools
```

## Test Build and Execution

For you go into your iceoryx_workspace folder and do the colcon build:
```bash
colcon build
```
Expected output should be like this: `Summary: 13 packages finished [24.1s]`
Colcon automatically creates the folders `build`, `install` and `log`.

**_NOTE:_** If you get an build error related to CycloneDDS, please delete the build folder from the iceoryx directory.

For executing tests you can use colcon too:
```bash
colcon test --packages-select iceoryx_systemtest
```
For the case that a test fails the output look like this
```bash
--- stderr: iceoryx_systemtest                     
Errors while running CTest
---
Finished <<< iceoryx_systemtest [7.49s] [ with test failures ]

Summary: 13 packages finished [7.80s]
  1 package had stderr output: iceoryx_systemtest
  1 package had test failures: iceoryx_systemtest
```

For observing test logs in an error case you can use `colcon test-result`:
```
colcon test-result --all --verbose
```

With colcon all tests in the systemtest package are executed. In particular cases you want to execute only
one testfile. This can be done the following way:
```bash
source install/setup.bash # your install directory created by colcon
launch_test src/iceoryx/iceoryx_systemtest/iceoryx_systemtest/test_roudi_startup_shutdown.py 
```

An output can look like this:
```bash
    ======================================================================
    FAIL: test_roudi_ready (iceoryx_systemtest.TestRouDiProcess)
    ----------------------------------------------------------------------
    Traceback (most recent call last):
      File "iceoryx_ws/src/iceoryx/iceoryx_systemtest/iceoryx_systemtest/test_roudi_startup_shutdown.py", line 52, in test_roudi_ready
        proc_output.assertWaitFor(
      File "/opt/ros/foxy/lib/python3.8/site-packages/launch_testing/io_handler.py", line 146, in assertWaitFor
        assert success, 'Waiting for output timed out'
    AssertionError: Waiting for output timed out
```

## Open points
- use an alternative way of tracing test information of the test processes without involving iceoryx (e.g. DDS or some tracing lib)
- add gtest for detailed testing in the test processes.
