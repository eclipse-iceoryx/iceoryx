This document is a declaration of the software quality for the `iceoryx_hoofs`, `iceoryx_posh` and `iceoryx_binding_c` packages, based on the guidelines in [REP-2004](https://www.ros.org/reps/rep-2004.html). They are part of the Eclipse iceoryx middleware.

# Quality Declaration

The packages `iceoryx_hoofs`, `iceoryx_posh` and `iceoryx_binding_c` claim to be in the **Quality Level 2** category. In the following documentation, these packages are referred to `iceoryx`. Some of the rationales in this document refer to the complete project.

Below are the rationales, notes, and caveats for this claim, organized by each requirement listed in the [Package Requirements for Quality Level 2 in REP-2004](https://www.ros.org/reps/rep-2004.html).

See [the Readme](https://github.com/eclipse-iceoryx/iceoryx#quality-levels--platforms) for a full list of the quality levels of iceoryx components and [Contributing Guide](https://github.com/eclipse-iceoryx/iceoryx/blob/master/CONTRIBUTING.md#quality-levels) for a description of the quality levels.

## Version Policy [1]

### Version Scheme [1.i]

`iceoryx` follows the [semver](https://semver.org/#summary) specification for versioning. The `rmw` implementation [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx) follows the same rules.
Changes for MINOR releases are API compatible and ideally ABI stable if not otherwise clearly stated in the release notes

A similar format is prescribed by the Eclipse Foundation in the [Release Handbook](https://www.eclipse.org/projects/handbook/#release).

On Git, the tags have a `v` prefix before the version numbers. A [release script](https://github.com/eclipse-iceoryx/iceoryx/blob/master/tools/scripts/iceoryx_release.sh) shall ensure that version numbers are kept consistent for all packages.

### Version Stability [1.ii]

`iceoryx` is at a stable version, i.e. `>= 1.0.0`. The latest valid release can be found on the [release page](https://github.com/eclipse-iceoryx/iceoryx/releases) of iceoryx.

The change history can be found in the [release notes section](https://iceoryx.io/latest/release-notes).

### Public API Declaration [1.iii]

All symbols in the installed headers are considered part of the public API.

Except for the exclusions listed below, all installed headers are in the include directory of the package, headers in any other folders are not installed and considered private. Headers under the folder `internal` are not considered part of the public API and are subject to change without notice.

### API Stability Within a Released ROS Distribution [1.iv]/[1.vi]

`iceoryx` guarantees public API stability for MINOR and PATCH releases and can not guarantee public API stability for MAJOR releases.

Taking into account that `iceoryx` is integrated as a shared-memory transport layer in
[Eclipse CycloneDDS](https://github.com/eclipse-cyclonedds/cyclonedds) and in [rmw_iceoryx](https://github.com/ros2/rmw_iceoryx) it is necessary to schedule releases and do compatibility testing to avoid disruption towards ROS.

### ABI Stability Within a Released ROS Distribution [1.v]/[1.vi]

`iceoryx` aims to be ABI/API-stable for MINOR and PATCH releases for maintaining ROS releases.
For ROS integration are release branches with the same policies like the `master` branch maintained for long-term support.

## Change Control Process [2]

The Eclipse iceoryx project follows the recommended guidelines of the [Eclipse Development Process](https://www.eclipse.org/projects/dev_process/).

The Eclipse Foundation manages the write access to project repositories, allowing only designated [Committers](https://www.eclipse.org/projects/dev_process/#4_1_Committers), who have been voted for in elections overseen by the Eclipse Foundation, to commit changes.

API and ABI stability are part of the review process. The Eclipse iceoryx project runs CI and tests on multiple platforms with different compilers.

It is planned to build and test the changes on the ROS CI for early detection of breaking changes.

### Change Requests [2.i]

All changes in the codebase are done with a Pull-Request. It is mandatory to link the Pull-Request to a corresponding [issue ticket](https://github.com/eclipse-iceoryx/iceoryx/blob/master/CONTRIBUTING.md#feature-request-and-bugs) on GitHub to ensure traceability.
The contributor and reviewer are required to fill out a [Pull-Request Template](https://github.com/eclipse-iceoryx/iceoryx/blob/master/.github/PULL_REQUEST_TEMPLATE.md) before merging.

### Contributor Origin [2.ii]

This project uses DCO as its confirmation of contributor origin policy.
More information can be found in [Eclipse Foundation's DCO policy](https://www.eclipse.org/legal/DCO.php).
Eclipse projects furthermore require from the contributor that an [Eclipse Contributor Agreement](https://www.eclipse.org/legal/ECA.php) is on file with the Eclipse Foundation.

### Peer Review Policy [2.iii]

All pull requests must pass peer-review and can only be merged if two Committers (Maintainer) approve it.
Check [Eclipse Developer Process](https://www.eclipse.org/projects/dev_process/) for additional information.

### Continuous Integration [2.iv]

Pull requests are required to pass all jobs in the CI system before merging even when it is approved.
Additionally, the CI builds always the latest master branch to ensure functionality.

The results of the CI are made public and cover x64 platforms running Linux, macOS, and Windows:

Regularly tested platforms on CI:
[![Build & Test](https://github.com/eclipse-iceoryx/iceoryx/actions/workflows/build-test.yml/badge.svg)](https://github.com/eclipse-iceoryx/iceoryx/actions/workflows/build-test.yml)

These are run with a mixture of release, release with debug symbols, debug, and address sanitizer builds. All CI builds build and execute all tests, including system-level tests.

### Documentation Policy [2.v]

It is required to create/modify the Doxygen/design and user documentation within a Pull-Request if necessary and checked by reviewers.

## Documentation [3]

### Feature Documentation [3.i]

The documentation of the main iceoryx features (sending, receiving data) is done within the [overview](https://iceoryx.io/latest/getting-started/overview/) and [iceoryx examples](https://iceoryx.io/latest/getting-started/examples/) including a user-friendly description on how to use the iceoryx API.
The [configuration guide](https://iceoryx.io/latest/advanced/configuration-guide/) completes the documentation on how to use iceoryx.

Detailed technical documentation about iceoryx features can be found in the [design document](https://github.com/eclipse-iceoryx/iceoryx/tree/master/doc/design) section with descriptions and diagrams about internal mechanisms of iceoryx.

For new features, it is recommended to create first a design document, see the [Contribution Guidelines](https://github.com/eclipse-iceoryx/iceoryx/blob/master/CONTRIBUTING.md#feature-request-and-bugs) for more information.
Diagrams in feature descriptions need to be consistently generated from [PlantUML](https://github.com/eclipse-iceoryx/iceoryx/tree/master/doc/design#design-documents).

Currently, the committers are working for the completion of the documentation of existing features to reach quality level 1.

### Public API Documentation [3.ii]

The public API is documented in form of Doxygen comments and available as API reference on [iceoryx.io](https://iceoryx.io/).

### License [3.iii]

The license for Eclipse iceoryx is the Apache License 2.0, and all of the code includes a header stating that.
The full license text is available in the [`LICENSE`](https://github.com/eclipse-iceoryx/iceoryx/blob/master/LICENSE) file.
The project includes a [`NOTICE`](https://github.com/eclipse-iceoryx/iceoryx/blob/master/NOTICE.md) with links to more information about these licenses.

There is some third-party content included with Eclipse iceoryx which is licensed as MIT or New BSD.
Details can also be found in the included [`NOTICE`](https://github.com/eclipse-iceoryx/iceoryx/blob/master/NOTICE.md#third-party-dependencies) document.

### Copyright Statement [3.iv]

Each source code file in Eclipse iceoryx has a copyright header that needs to follow this [style](https://github.com/eclipse-iceoryx/iceoryx/blob/master/CONTRIBUTING.md#header).

A CI job ensures by checking with `ament_copyright` that all files comply with this rule.

## Testing [4]

Every iceoryx package has a `test` folder that contains subfolders for [module- and/or integrationtests](https://github.com/eclipse-iceoryx/iceoryx/blob/master/CONTRIBUTING.md#testing) written based on the Google test Framework. All tests are running on the CI for every Pull-Request and merge to master for all supported platforms.
Currently, due to limited support on Windows, there are some exclusions for testing on the CI.

### Feature Testing [4.i]

The features of iceoryx are tested by the [iceoryx_integrationtest](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_integrationtest) package. Using `launch_test` are tests on system-level executed to ensure that the iceoryx packages are functional and the Public API usable.
There is continuous effort ongoing to cover the corner cases in the usage of iceoryx.

### Public API Testing [4.ii]

For every major feature all are tests executed. New features must provide unit and integration tests that cover the code changes in the Pull-Request. The tests reside in separated folders for every package following a defined structure and naming convention.
The features are tested on module(unit) -integration and system test level. [Guidelines](https://iceoryx.io/latest/advanced/best-practice-for-testing/) for Contributors ensure high quality for test development.

### Coverage [4.iii]

The Eclipse iceoryx project use [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html) to capture the following coverage types:

- Line Coverage
- Function Coverage
- Branch Coverage

The coverage results of every Pull-Request and master branch are publicly available on [codecov.io](https://app.codecov.io/gh/eclipse-iceoryx/iceoryx).
A detailed report (e.g. the coverage in different packages) can be obtained by following [this](https://github.com/eclipse-iceoryx/iceoryx/blob/master/CONTRIBUTING.md#coverage-scan) guide.

### Performance [4.iv]

The most important measurement units for performance testing on iceoryx are the data transfer latency and the CPU load.
They can be obtained in two ways:

1. [iceperf](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/iceperf#iceperf)
This performance test uses the iceoryx API directly without any indirections to compare different IPC mechanisms on increasing payload sizes. The output is a table linking the payload sizes with the latency in microseconds.

2. [performance_test](https://gitlab.com/ApexAI/performance_test/-/tree/master/)
The `performance_test` package tests various middleware implementations in ROS using inter-process communication.
It offers a communication binding for iceoryx and gives information about latency, CPU usage etc.
Additionally, it is possible to test here the integration of iceoryx + CycloneDDS in ROS using the rmw communication abstraction.

Currently, there are no automatic regression tests in the CI running. The work to integrate this is addressed in [https://github.com/eclipse-iceoryx/iceoryx/issues/137](https://github.com/eclipse-iceoryx/iceoryx/issues/137). The performance tests are currently executed manually before every release that will be integrated into ROS by the maintainers to avoid performance regression.

### Linters and Static Analysis [4.v]

The code formatting is enforced through the CI using `clang-format` and can be used by following this [guide](https://github.com/eclipse-iceoryx/iceoryx/blob/master/CONTRIBUTING.md#coding-style)

The codebase is linted with `clang-tidy` to enforce naming conventions and basic coding rules and checked in the Pull-Requests by review.
A [static code analysis](https://github.com/eclipse-iceoryx/iceoryx/blob/master/CONTRIBUTING.md#static-code-analysis) is done regularly with the Axivion Suite, but the results are not publicly accessible due to licensing.

## Dependencies [5]

### Direct Runtime ROS Dependencies [5.i]

As an external dependency, there are no ROS dependencies in `iceoryx_hoofs`, `iceoryx_posh`, and `iceoryx_binding_c`.

### Optional Direct Runtime ROS Dependencies [5.ii]

As an external dependency, there are no ROS dependencies in `iceoryx_hoofs`, `iceoryx_posh`, and `iceoryx_binding_c`.

### Direct Runtime non-ROS Dependency [5.iii]

Besides a POSIX compliant OS, the only runtime dependency is the [Access Control List](https://en.wikipedia.org/wiki/Access-control_list) support that can be installed within the system packages.
The underlying filesystem needs to support ACL.

## Platform Support [6]

Eclipse iceoryx supports the Tier 1 platforms as described in [REP-2000](https://www.ros.org/reps/rep-2000.html).
Every release that is integrated with ROS will be built in the ROS CI to ensure compatibility.

## Security [7]

### Vulnerability Disclosure Policy [7.i]

This package conforms to the Vulnerability Disclosure Policy in REP-2006.
The Eclipse Project Handbook states the project's [vulnerability disclosure policy](https://www.eclipse.org/projects/handbook/#vulnerability-disclosure) in detail.

On the [iceoryx website](https://iceoryx.io/) one finds a link to report a security vulnerability available.
