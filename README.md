# iceoryx - true zero-copy inter-process-communication

<p align="center">
<img src="https://user-images.githubusercontent.com/8661268/114321508-64a6b000-9b1b-11eb-95ef-b84c91387cff.png" width="50%">
</p>

[![Build & Test](https://github.com/eclipse-iceoryx/iceoryx/workflows/Build%20&%20Test/badge.svg?branch=master)](https://github.com/eclipse-iceoryx/iceoryx/actions)
[![Integrationtests](https://github.com/eclipse-iceoryx/iceoryx/workflows/Iceoryx%20Integrationtests/badge.svg?branch=master)](https://github.com/eclipse-iceoryx/iceoryx/actions)
[![Gitter](https://badges.gitter.im/eclipse-iceoryx/iceoryx.svg)](https://gitter.im/eclipse/iceoryx)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0)
[![Codecov](https://codecov.io/gh/eclipse-iceoryx/iceoryx/branch/master/graph/badge.svg?branch=master)](https://codecov.io/gh/eclipse-iceoryx/iceoryx?branch=master)
[![Sanitize](https://github.com/eclipse/iceoryx/workflows/Sanitize/badge.svg?branch=master)](https://github.com/eclipse/iceoryx/actions?query=workflow%3ASanitize)

## Introduction

Great that you've made it to this neat Eclipse project! Let's get you started by providing a quick background
tour, introducing the project scope and all you need for installation and a first running example.

So first off: What is iceoryx?

iceoryx is an inter-process-communication (IPC) middleware for various operating systems (currently we support Linux,
macOS, QNX, FreeBSD and Windows 10).
It has its origins in the automotive industry, where large amounts of data have to be transferred between different processes
when it comes to driver assistance or automated driving systems. However, the efficient communication mechanisms can also be applied
to a wider range of use cases, e.g. in the field of robotics or game development.

<p align="center">
<img src="https://user-images.githubusercontent.com/8661268/74612998-b962bc80-510a-11ea-97f0-62f41c5d287b.gif" width="100%">
</p>

iceoryx uses a true zero-copy, shared memory approach that allows to transfer data from publishers to subscribers without a single copy.
This ensures data transmissions with constant latency, regardless of the size of the payload. For more information have a look at the
[1000 words iceoryx introduction](https://www.eclipse.org/community/eclipse_newsletter/2019/december/4.php).

<p align="center">
<img src="https://user-images.githubusercontent.com/56729607/157733625-e3e61617-2f72-46ba-b4f2-5a3973f9ad6f.png" width="80%">
</p>

You're right, middleware is a cluttered term and can somehow be all or nothing. To get a better impression what
this means for iceoryx, please have a look at our [goals and non-goals](doc/goals-non-goals.md).

Don't get too frightened of the API when strolling through the examples. Think of the untyped C++ and the C API as a
"plumbing" one ("plumbing" as defined in Git, which means low-level). We're not using the "plumbing" APIs ourselves, but
instead the typed C++ API. The normal use case is that iceoryx is integrated as high-performance IPC transport layer in
a bigger framework with additional API layers.
An example for such a "porcelain" API would be [ROS 2](https://www.ros.org/).

You can find the full API documentation on 🌐 [https://iceoryx.io](https://iceoryx.io). <!--NOLINT explicit link to website-->

### Supported Platforms

Modification or addition of new platforms is described in the
[custom iceoryx platforms](doc/website/advanced/custom-iceoryx-platform.md) article.

|Operating System| Compiler  | supports access rights for shared memory | command line parsing    |
|----------------|-----------|:----------------------------------------:|:-----------------------:|
| Linux          | gcc/clang | yes                                      | yes                     |
| QNX            | gcc       | yes                                      | yes                     |
| MacOS          | clang     | no, not planned for implementation       | yes                     |
| Windows 10     | msvc      | no, not planned for implementation       | will be implemented     |
| FreeBSD (Unix) | clang     | no, not planned for implementation       | yes                     |

In general unix platforms should work with iceoryx but we only test FreeBSD on our CI.

### Where is Eclipse iceoryx used?

|Framework | Description |
|---|---|
| [ROS 2](https://github.com/ros2/rmw_iceoryx) | Eclipse iceoryx can be used inside the [Robot Operating System](https://www.ros.org/) with [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx.git) |
| [Eclipse eCAL](https://eclipse-ecal.github.io/ecal/) | Open-source framework from [Continental AG](https://www.continental.com/) supporting pub/sub and various message protocols |
| [RTA-VRTE](https://www.etas.com/en/products/rta-vrte.php) | [AUTOSAR Adaptive Platform](https://www.autosar.org/standards/adaptive-platform/) software framework for vehicle computer from [ETAS GmbH](https://www.etas.com) |
| [Cyclone DDS](https://github.com/eclipse-cyclonedds/cyclonedds) | Performant and robust open-source DDS implementation maintained by [ZettaScale Technology](https://zettascale.tech/) |
| [Apex.Ida from Apex.AI](https://www.apex.ai/apexida) | Safe and certified middleware for autonomous mobility systems from [Apex.AI](https://www.apex.ai/) |
| [AVIN AGNOSAR Adaptive Platform](https://www.avinsystems.com/products/agnosar-adaptive-platform/) | AUTOSAR Adaptive Platform Product from AVIN Systems |

## Build and install

You can find the build and installation guidelines [here](doc/website/getting-started/installation.md).

## Examples

After you've built all the necessary things, you can continue playing around with the [examples](./iceoryx_examples/README.md).

## Build and run in a Docker environment

If you want to avoid installing anything on your host machine but you have Docker installed, it is possible to use it to build and run iceoryx applications.

Please see the dedicated [README.md](tools/docker/README.md) for information on how to do this.

## Documentation

* [Getting Started](doc/website/getting-started/overview.md)
* [Installation Guide](doc/website/getting-started/installation.md)
* [iceoryx Hoofs Hacker Guide](iceoryx_hoofs/README.md)

### Quality levels & platforms

> [Quality level](./CONTRIBUTING.md#quality-levels) are 5 to 1+, where 1+ is the highest level.

Please see the [Quality Declaration](./QUALITY_DECLARATION.md) for details of the quality measures according to ROS 2 guidelines.

|CMake project/target   | Current Level | Target Level QNX  | Target Level <br> Linux, Windows, macOS |
|-----------------------|:-------------:|:-----------------:|:---------------------------------------:|
| iceoryx_hoofs         | 2             | 1+                | 1                                       |
| iceoryx_posh          | 2             | 1+, 2             | 1                                       |
| iceoryx_dust          | 2             | 2                 | 2                                       |
| iceoryx_binding_c     | 2             | 2                 | 2                                       |
| iceoryx_examples      | 5             | 4                 | 4                                       |
| iceoryx_introspection | 5             | 4                 | 4                                       |
| iceoryx_meta          | 5             | 5                 | 5                                       |

Is something missing or you've got ideas for other nifty examples? Jump right away to the next section!

## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) for a quick read-up about what to consider if you want to contribute.

## Planned features

Get to know the upcoming features and the project scope in [PLANNED_FEATURES.md](./PLANNED_FEATURES.md).

## Bindings and innovations enabled by iceoryx

|Name                                                                                 | Description                                                      | Technologies |
|-------------------------------------------------------------------------------------|------------------------------------------------------------------|--------------|
| [iceoryx-rs](https://github.com/eclipse-iceoryx/iceoryx-rs)                         | Rust binding for iceoryx                                         | Rust         |
| [iceoryx-automotive-soa](https://github.com/eclipse-iceoryx/iceoryx-automotive-soa) | Binding for automotive frameworks like AUTOSAR Adaptive ara::com | C++          |
| [iceoryx-gateway-dds](https://github.com/eclipse-iceoryx/iceoryx-gateway-dds)       | Gateway for Cyclone DDS                                          | DDS          |
| [iceray](https://github.com/elBoberido/iceray)                                      | An iceoryx introspection client written in Rust                  | Rust         |
| [Larry.Robotics](https://gitlab.com/larry.robotics)                                 | An iceoryx demonstrator for tinker, thinker and toddler          | Demonstrator |

## Frequently Asked Questions (FAQ)

[FAQ.md](./doc/website/FAQ.md)

## Governance & maintainers

Please have a look at the [GOVERNANCE.md](./GOVERNANCE.md).
