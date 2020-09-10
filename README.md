# iceoryx - true zero-copy inter-process-communication

<p align="center">
<img src="https://user-images.githubusercontent.com/8661268/70233652-4aa6d180-175f-11ea-8524-2344e0d3935c.png" width="50%">
</p>

[![Build & Test](https://github.com/eclipse/iceoryx/workflows/Build%20&%20Test/badge.svg?branch=master)](https://github.com/eclipse/iceoryx/actions)
[![Gitter](https://badges.gitter.im/eclipse/iceoryx.svg)](https://gitter.im/eclipse/iceoryx)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Codecov](https://codecov.io/gh/eclipse/iceoryx/branch/master/graph/badge.svg?branch=master)](https://codecov.io/gh/eclipse/iceoryx?branch=master)

## Introduction

Great that you've made it to this little Eclipse project! Let's get you started by providing a quick background
tour, introducing the project scope and all you need for installation and a first running example.

So first off: What is iceoryx?

iceoryx is an inter-process-communication (IPC) middleware for various operating systems (currently we support Linux, MacOS and QNX).
It has its origins in the automotive industry, where large amounts of data have to be transferred between different processes
when it comes to driver assistance or automated driving systems. However, the efficient communication mechanisms can also be applied
to a wider range of use cases, e.g. in the field of robotics or game development. 

<p align="center">
<img src="https://user-images.githubusercontent.com/8661268/74612998-b962bc80-510a-11ea-97f0-62f41c5d287b.gif" width="100%">
</p>

iceoryx uses a true zero-copy, shared memory approach that allows to transfer data from publishers to subscribers without a single copy. 
This ensures data transmissions with constant latency, regardless of the size of the payload. For more information have a look at the 
[1000 words iceoryx introduction](https://www.eclipse.org/community/eclipse_newsletter/2019/december/4.php)

<p align="center">
<img src="https://user-images.githubusercontent.com/55156294/91751530-35af7f80-ebc5-11ea-9aed-eed590b229df.png" width="80%">
</p>

You're right, middleware is a cluttered term and can somehow be all or nothing, so let's talk about the [goals and non-goals](doc/goals-non-goals.md) of iceoryx.

It's all about the API?!

Don't get too frighten of the API when strolling through the codebase. Think of iceoryx's API as a "plumbing" one
("plumbing" as defined in Git, which means low-level). We're not using the "plumbing" API ourselves, but instead a typed API.
The normal use case is that iceoryx is integrated as high-performance IPC transport layer in a bigger framework with additional API layers. 
An example for such a "porcelain" API would be [ROS2](https://www.ros.org/). Others are listed in the next section.

### Where is Eclipse iceoryx already used?

|Framework | Description |
|---|---|
| [ROS2](https://github.com/ros2/rmw_iceoryx) | Eclipse iceoryx can be used inside the [robot operating system](https://www.ros.org/) with [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx.git) |
| [eCAL](https://github.com/continental/ecal) | Open-source framework from [Continental AG](https://www.continental.com/) supporting pub/sub and various message protocols |
| [RTA-VRTE](https://www.etas.com/en/products/rta-vrte.php) | [Adaptive AUTOSAR](https://www.autosar.org/standards/adaptive-platform/) platform software framework for vehicle computer from [ETAS GmbH](https://www.etas.com) |
| [Cyclone DDS](https://github.com/eclipse-cyclonedds/cyclonedds) | Performant and robust open-source DDS implementation maintained by [ADLINK Technology Inc.](https://www.adlinktech.com/) |

## Build and install

You can find the build and installation guidelines [here](doc/installation-guide.md).

## Examples

After you've built all the necessary things, you can continue playing around with the [examples](./iceoryx_examples).

## Build and run in a Docker environment

If you want to avoid installing anything on your host machine but you have Docker installed, it is possible to use it to build and run iceoryx applications.

Please see the dedicated [README.md](tools/docker/README.md) for information on how to do this.

## Documentation

* [Concepts](doc/conceptual-guide.md)
* [Usage Guide](doc/usage-guide.md)
* [Ice0ryx Utils Hacker Guide](iceoryx_utils/README.md)

### Targeted quality levels & platforms

> [Quality level](./CONTRIBUTING.md#quality-levels) are 5..1, where 1 is highest

|CMake project/target                     | QNX  | Linux, Windows, MacOS | Comment                             |
|-----------------------------------------|:----:|:---------------------:|:-----------------------------------:|
| example_benchmark_optional_and_expected | 5    | 5                     |                                     |
| example_icedelivery                     | 5    | 5                     |                                     |
| example_icedelivery_on_c                | 5    | 5                     |                                     |
| example_iceperf                         | 5    | 5                     |                                     |
| example_singleprocess                   | 5    | 5                     |                                     |
| iceoryx_binding_c                       | 5    | 5                     | Not final and can change in the near future |
| iceoryx_dds                             | 4    | 4                     |                                     |
| iceoryx_meta                            | 5    | 5                     |                                     |
| iceoryx_posh                            | 1, 2 | 4                     | Will be split into separate targets |
| iceoryx_utils                           | 1    | 4                     |                                     |
| iceoryx_introspection                   | 5    | 5                     |                                     |

## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) for a quick read-up about what to consider if you want to contribute.

## Planned features

Get to know the upcoming features and the project scope in [PLANNED_FEATURES.md](./PLANNED_FEATURES.md).

## Innovations enabled by iceoryx

|Name | Description | Technologies |
|---|---|---|
| [Larry.Robotics](https://gitlab.com/larry.robotics) | An iceoryx demonstrator for tinker, thinker and toddler | Demonstrator |
| [iceoryx-rs](https://github.com/elBoberido/iceoryx-rs) | Experimental Rust wrapper for iceoryx | Rust |
| [IceRay](https://github.com/elBoberido/iceray) | An iceoryx instrospection client written in Rust | Rust |

Is something missing or you've got ideas for other nifty examples? Jump right away to the next section!

## Maintainers

* Michael Pöhnl (michael.poehnl@de.bosch.com)
* Christian Eltzschig (christian.eltzschig2@de.bosch.com)
* Dietrich Krönke (dietrich.kroenke2@de.bosch.com)
* Mathias Kraus (mathias.kraus2@de.bosch.com)
* Matthias Killat (matthias.killat2@de.bosch.com)
* Martin Hintz (martin.hintz@de.bosch.com)
* Simon Hoinkis (simon.hoinkis2@de.bosch.com)
