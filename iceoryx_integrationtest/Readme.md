# Iceoryx integration tests

## Introduction

To ensure quality standards in iceoryx, we are using automated testing to ensure ISO26262 on unit test and integration
test level of the respective components. Additionally, we need to make sure that the customer-facing API is functional
and software integration specifications are fulfilled. Test focus is here on SWE.5 according to ASPICE.

For that purpose we bring in tests which simulate customer behavior to have automatic testing of (Mis)Use-cases.
As testing framework we use the [launch_testing](https://github.com/ros2/launch/blob/humble/launch_testing/README.md) framework from ROS 2.
The tests are stored in a ROS 2 CMake package where the test executables are built and launched/tested by python scripts.
In these scripts the testing is currently done by evaluating the stdout output of the processes and the exit codes.

Advantages:

- automatic test if processes have exited unexpectedly
- test against custom or error return codes to evaluate error-cases
- test the order of printed messages

Limitations:

- testing against stdout is error prone
- limited functionality for performance testing because the stdout is buffered (messages could be reordered)

## Setup

To build and execute the tests you need to have ROS 2 installed. Please follow the instructions on <https://docs.ros.org/en/humble/Installation.html>.
The system tests are currently tested on ROS 2 "Humble Hawksbill" in Ubuntu 22.04 LTS.

Please remove beforehand the `COLCON_IGNORE` files from `iceoryx_integrationtest` and `iceoryx_examples`.

For a basic setup you need to install the following packages:

```bash
sudo apt install ros-humble-ros-base ros-humble-launch-testing ros-humble-ament-cmake python3-colcon-common-extensions
```

For the future versions you can use the corresponding ROS 2 release.

Once installed, you need to source ROS 2 to make the environment available in your terminal:

```bash
source /opt/ros/humble/setup.bash
```

**_NOTE:_** You can add the source command to your `~/.bashrc` for automatic loading the ROS 2 workspace at boot time.

Required for the colcon build of iceoryx is that the repository is located within a ROS workspace like this:

```shell
iceoryx_workspace
└── src
    └── iceoryx
        ├── cmake
        ├── doc
        ├── iceoryx_binding_c
        ├── iceoryx_examples
        ├── iceoryx_hoofs
        ├── iceoryx_integrationtest
        ├── iceoryx_meta
        ├── iceoryx_posh
        └── tools
```

## Test Build and Execution

Go into your iceoryx_workspace folder and do the colcon build:

```bash
colcon build --packages-up-to iceoryx_integrationtest --meta=src/iceoryx/iceoryx_integrationtest/colcon.meta
```

The `colcon.meta` sets the required cmake parameter to also build and test the examples with the experimental API.

The expected output should be like this: `Summary: 21 packages finished [31.7s]`
Colcon automatically creates the folders `build`, `install` and `log`.

**_NOTE:_** Please make sure that the build folder in src/iceoryx is not present to avoid conflicts with the
colcon build.

For executing tests you can use colcon too:

```bash
source install/setup.bash # your install directory created by colcon
colcon test --packages-select iceoryx_integrationtest --meta=src/iceoryx/iceoryx_integrationtest/colcon.meta
```

In case a test fails, the output looks like this

```bash
--- stderr: iceoryx_integrationtest
Errors while running CTest
---
Finished <<< iceoryx_integrationtest [4min 50s] [ with test failures ]

Summary: 1 package finished [4min 50s]
  1 package had stderr output: iceoryx_integrationtest
  1 package had test failures: iceoryx_integrationtest
```

You can use `colcon test-result` to obtain test logs in an error case:

```bash
colcon test-result --all --verbose
```

With colcon all tests in the iceoryx_integrationtest package are executed. In particular cases you may
want to execute only one test file. This can be done in the following way:

```bash
launch_test src/iceoryx/iceoryx_integrationtest/iceoryx_integrationtest/test_roudi_startup_shutdown.py
```

An output for a failing test could look like this:

```bash
    ======================================================================
    FAIL: test_roudi_ready (iceoryx_integrationtest.TestRouDiProcess)
    ----------------------------------------------------------------------
    Traceback (most recent call last):
      File "iceoryx_workspace/src/iceoryx/iceoryx_integrationtest/iceoryx_integrationtest/test_roudi_startup_shutdown.py", line 52, in test_roudi_ready
        proc_output.assertWaitFor(
      File "/opt/ros/humble/lib/python3.8/site-packages/launch_testing/io_handler.py", line 146, in assertWaitFor
        assert success, 'Waiting for output timed out'
    AssertionError: Waiting for output timed out
```

## Open points

- use an alternative way of tracing test information of the test processes without involving iceoryx (e.g. DDS or some tracing lib)
- add gtest for detailed testing in the test processes
