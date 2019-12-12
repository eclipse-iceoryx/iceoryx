# iceoryx - an IPC middleware for POSIX-based systems

<p align="center">
<img src="https://user-images.githubusercontent.com/8661268/70233652-4aa6d180-175f-11ea-8524-2344e0d3935c.png" width="50%">
</p>

## Introduction

Great that you've made it to this little Eclipse project! Let's get you started by providing a quick background
tour, introducing the project scope and guide you through the examples.

So first off: What is iceoryx?

Iceoryx is an inter-process communication (IPC) middleware for [POSIX](https://en.wikipedia.org/wiki/POSIX) based
operating systems. It features shared memory capabilities which allow a true zero-copy data transfer.

Originating from the automotive domain, it is crucial to transfer a huge amount of data between multiple processes to
realize driver assistance systems or automated driving applications. Moreover, the same efficient communication
mechanism can be applied to a broader range of use cases, e.g. in the field of robotics or game development.

It's all about the API!

Don't get too frighted of the API when strolling through the codebase. Think of iceoryx's API as a "plumbing" one
("plumbing" as defined in Git, which means low-level). We're not using the "plumbing" API ourselves, but instead a typed API.
Examples for a "porcelain" API would be e.g.
[Adaptive Autosar Foundation](https://www.autosar.org/fileadmin/Releases_TEMP/Adaptive_Platform_19-03/AdaptiveFoundation.zip)
(see AUTOSAR_EXP_ARAComAPI.pdf) or [ROS](https://www.ros.org).

### Scope

Who can benefit of using iceoryx? What's in for those folks?

#### User personas

**Andrew, the HAD developer**
Andrew is a software developer at a startup working on autonomous cars. Currently their project is using ROS, because
it's easy to get the car driving. After some months he's realizing that sending gigabytes around, leads to high runtime
demands with ROS. A college mentions iceoryx during lunch, which might be interesting because it has a zero-copy
mechanism and offers a ROS RMW implementation. Soon after giving iceoryx a try, Andrew is thrilled about it. He can not only feel
the runtime performance boost, but also still keep using his beloved ROS visualization tools!

**Martha, the indie game developer**
Martha always had troubles with those silly game engines. Some are slow but free, others are fast but too expensive.
It's a hard life if you're independent. When a friend who works in the automotive industry mentions he has just started
using iceoryx, which offers fast shared memory communication she listens up. Iceoryx is soley passing pointers around
and does avoid copies to the utmost? "I'll definitely try iceoryx in my new project and see if I can speed up the
performance with my low cost engine" she thinks while wandering home at night after the meetup with her friend.

**Robby, the lonely robot**
Robby is autonomous robot built during a research project at a university. He has a great set of features and can
astonish the crowds by creating a detailed map of the university building in under an hour. However they made him use
that slow self-made IPC to communicate with his sensors, because his parents wanted to get started fast. Though that
makes it hard for him to react in real-time to dangerous incidents like flying coffee cups. When strolling through
the interwebs on a lonely evening, he finds out about iceoryx: Free-to-use, high-performance data transfer with low
runtime overhead, real-time support! Brilliant! Maybe even Robbys biggest wish for a network binding will come true,
so he can stream his favorite [video](https://www.youtube.com/watch?v=g5NkgZXWl0w) even faster!

## Download

<!--
Either download our pre-built daemon (called RouDi) and our runtime lib or build everything yourself.

### Release

    wget iceoryx.deb
    dpkg -i iceoryx.deb

You've sucessfully installed iceoryx! Continue with the section examples to see how to get started developing applications using iceoryx.
-->

### Development

Great that you want start developing iceoryx! In order to get started please consider the next sections.

#### Prerequisites

Altough we strive to be fully POSIX-compliant, we recommend to use Ubuntu 18.04 and at least GCC 7.4.0 for development.

You will need to install the following packages:

    sudo apt-get install cmake libacl1-dev libncurses5-dev pkg-config

#### Build from sources

iceoryx_utils and iceoryx_posh are deployed as independent cmake packages. Posh is using some functions from utils and is depending on it. You are able to build posh and utils and integrate in into existong cmake projects.

##### Build Script
For the first start we advise to use our build-test script for building everything.

    git clone https://github.com/eclipse/iceoryx.git
    ./tools/iceoryx_build_test.sh

In default-mode the script is not building the provided test. For a clean build just add "clean" as first argument to the script.

##### Build with tests

To build iceoryx with tests, just add "test" as first argument to the script.

    ./tools/iceoryx_build_test.sh test

The Googletest-Framework will be automatically fetched from github and the test will be executed and the end of the script.

##### Build with colcon

Alternatively, iceoryx can be built with [colcon](https://colcon.readthedocs.io/en/released/user/installation.html) to provide a smooth integration for ROS2 developers.

```
mkdir -p iceoryx_ws/src
cd $_
git clone https://github.com/eclipse/iceoryx.git
cd ..
colcon build
```

This build method makes the most sense in combination with [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx.git)

Congrats! You've build all the necessary things to continue playing around with the examples.

##### Build and run in a Docker environment

If you want to avoid installing anything on your host machine but you have Docker installed, it is possible to use it to build and run iceoryx applications.

Please see the dedicated [README.md](tools/docker/README.md) for information on how to do this.

## Examples

Great that you're still here, time for code! The following examples give you a quick and easy introduction into the inner
workings of iceoryx. We hope you enjoy our sightseeing tour!

|Name | Description | Technologies |
|---|---|---|
| [icedelivery](./iceoryx_examples/icedelivery) | Transfer data between POSIX applications | [SoA](https://en.wikipedia.org/wiki/Service-oriented_architecture), service description |
| [iceperf](./iceoryx_examples/iceperf) | Measure the latency from publisher to subscriber | Latency |
| [icecrystal](./iceoryx_examples/icecrystal) | Learn how to use the introspection | Debugging |
| [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx) | See how iceoryx can be used inside the robot operating system | ROS, RMW |

Is something missing or you've got ideas for other nifty examples? Jump right away to the next section!

## Contribute

Please refer to the [CONTRIBUTING.md](./CONTRIBUTING.md) for a quick read-up about what to consider if you want to contribute.

### Planned features

Get to know the upcoming features and the project scope in [PLANNED_FEATURES.md](./PLANNED_FEATURES.md).

## Maintainers

* Michael Pöhnl (michael.poehnl@de.bosch.com)
* Christian Eltzschig (christian.eltzschig2@de.bosch.com)
* Dietrich Krönke (dietrich.kroenke2@de.bosch.com)
* Mathias Kraus (mathias.kraus2@de.bosch.com)
* Matthias Killat (matthias.killat2@de.bosch.com)
* Martin Hintz (martin.hintz@de.bosch.com)
* Simon Hoinkis (simon.hoinkis2@de.bosch.com)
