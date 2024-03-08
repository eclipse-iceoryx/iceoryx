# Installation guide for contributors

## Build and run tests

While developing on iceoryx, you may want to know if your changes will break existing functionality or if your
newly written tests will pass. For that purpose, we generate CMake targets that execute the tests. First,
we need to build them:

```bash
cmake -Bbuild -Hiceoryx_meta -DBUILD_TEST=ON
cmake --build build
```

CMake automatically installs GoogleTest as a local dependency and builds the tests against it. Please note that
if you want to build tests for extensions like the C binding you need to enable this extension as well in the
CMake build. To build the tests for all extensions simply add `-DBUILD_ALL` to the CMake command.

!!! hint
    Before creating a Pull-Request, you should check your code for compiler warnings. The `-DBUILD_STRICT` CMake option
    is available for this purpose, which treats compiler warnings as errors. This flag is enabled on the GitHub
    CI for building Pull-Requests.

Now let's execute all tests:

```bash
cd iceoryx/build
make all_tests
```

Some of the tests are time-dependent and need a stable environment. These timing tests are available in separate targets:

```bash
make timing_module_tests
make timing_integration_tests
```

In iceoryx we distinguish between different test levels. The most important are: Module tests and Integration tests.
Module or unit tests are basically black box tests that test the public interface of a class.
In integration tests the interaction of several classes is tested.
The source code of the tests is placed into the folder `test` within the different iceoryx components. You can find there at least a folder `moduletests` and sometimes `integrationtests`.

If you now want to create a new test, you can place the source file directly into the right folder. CMake will
automatically detect the new file when doing a clean build and will add it to the corresponding executable.
There is no need to add a gtest main function because we already provide it. Executables are created for every test
level, for example `posh_moduletests`. They are placed into the corresponding build folder (e.g. `iceoryx/build/posh/test/posh_moduletests`).

If you want to execute only individual test cases, you can use these executables together with a filter command.
Let's assume you want to execute only `ServiceDescription_test` from posh_moduletests:

```bash
./build/posh/test/posh_moduletests --gtest_filter="ServiceDescription_test*"
```

!!! hint
    While writing code on iceoryx you should use git hooks that automatically ensure that you follow the coding and style guidelines.
    See [`git-hooks`](../../../tools/git-hooks/Readme.md).

## Use Sanitizer Scan

Due to the fact that iceoryx works a lot with system memory, it should be ensured that errors like memory leaks are not introduced.
To prevent this, we use the clang toolchain which offers several tools for scanning the codebase. One of them is the
[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) which checks e.g. for dangling pointers.

The below-listed sanitizers are enabled at the moment.

- [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) (ASan) is a fast memory error detector.
!!! note
    AddressSanitizer exits on the first detected error, which means there could be more errors in the codebase when this error is reported.
- [LeakSanitizer](https://clang.llvm.org/docs/LeakSanitizer.html) (LSan) is a run-time memory leak detector. In iceoryx, it runs as part of the AddressSanitizer.
- [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html) (UBSan) is a fast undefined behavior detector. iceoryx uses default behavior i.e. `print a verbose error report and continue execution`

With the `iceoryx_build_test.sh` script you can run the scan yourself. Additionally, the scans are running on the CI in every Pull-Request.
As a prerequisite, you need to install the clang compiler:

```bash
sudo apt install clang
```

Then you need to compile iceoryx with the sanitizer flags:

```bash
./tools/iceoryx_build_test.sh build-strict build-all sanitize clang clean
```

Now we can run the tests with enabled sanitizer options:

```bash
cd build && ./tools/run_tests.sh
```

If errors occur, an error report is shown with a stack trace to find the place where the leak occurs. If the leak
has its origin in an external dependency or shall be handled later then it is possible to set a function on a suppression list.
This should be used only rarely and only in coordination with an iceoryx maintainer.

!!! note
    iceoryx needs to be built as a static library to work with sanitizer flags, which is automatically achieved when using
    the script. If you want to use the ${ICEORYX_CXX_WARNINGS} then you have to call `find_package(iceoryx_platform)` and
    `include(IceoryxPlatformSettings)` into your cmake project and `include(IceoryxPlatform)` to make use of the ${ICEORYX_SANITIZER_FLAGS}.

## iceoryx library build

The iceoryx build consists of several libraries which have dependencies on each other. The goal is to have encapsulated
library packages available so that the end-user can easily find them with the CMake command `find_package(...)`.
In the default case, the iceoryx libraries are installed by `make install` into `/usr/lib` which requires root access.
As an alternative you can install the libs into a custom folder by setting `-DCMAKE_INSTALL_PREFIX=/custom/install/path`
as build flag for the CMake file in iceoryx_meta.

iceoryx_meta collects all libraries (hoofs, posh etc.) and extensions (binding_c) and can be a starting point for
the CMake build. The provided build script `tools/iceoryx_build_test.sh` uses iceoryx_meta.

Per default, iceoryx is built as static lib for better usability.
Additionally, we offer to build as shared library because it is a cleaner solution for resolving dependency issues and it reduces the linker time.
This is done by the flag `BUILD_SHARED_LIBS` which is set to OFF per default in iceoryx_meta. If you want to have shared libraries, just pass `-DBUILD_SHARED_LIBS=ON` to CMake or use `build-shared` as a flag in the build script.

!!! note
    When building with `colcon` in ROS 2, the packages `iceoryx_hoofs`, `iceoryx_posh` and `iceoryx_binding_c` are built
    automatically as shared libraries.

If iceoryx builds shared libraries you have to copy them into a custom path and set the LD_LIBRARY_PATH to the custom path (e.g. build/install/prefix).

```bash
export LD_LIBRARY_PATH=/your/path/to/iceoryx/libs
```

or you can set it directly:

```bash
LD_LIBRARY_PATH=/your/path/to/lib iox-roudi
```

If you want to share iceoryx to other users, you can create a debian package. This can be done by using: `./tools/iceoryx_build_test.sh package` where it will be deployed into the `build_package` folder.

!!! note
    The CMake libraries export their dependencies for easier integration. This means that you do not need to do a `find_package()`
    for all the dependencies. For example, you don't need to call `find_package(iceoryx_hoofs)` when you already called
    `find_package(iceoryx_posh)` since iceoryx_posh includes iceoryx_hoofs.

## Tips & Tricks

Sometimes one can encounter a failing CI target which is not reproducible locally on the developer
machine. With `./tools/scripts/ice_env.sh` one can create a
docker container with preinstalled dependencies and a configuration similar to
the CI target container.

When for instance the target Ubuntu 18.04 fails we can start the container with

```sh
./tools/scripts/ice_env.sh enter ubuntu:18.04
```

which enters the environment automatically and one can start debugging.

## Bazel

When working with Bazel, additional tools can help the developer to maintain
a consistent codebase similar to the Clang Tools (clang-format and clang-tidy) for C++.
The [Buildifier Tool](https://github.com/bazelbuild/buildtools/blob/master/buildifier/README.md)
offers formatting and linting for Bazel files.

The formatting is based on rules given by Buildifier and the linting is based on
a list of [warnings](https://github.com/bazelbuild/buildtools/blob/master/WARNINGS.md).

To check formatting of the Bazel files the following command needs to run in the iceoryx
workspace.

```bash
cd iceoryx
bazel run //:buildifier
```

Buildifier automatically reformat the code.

For formatting and linting this command will do the job:

```bash
cd iceoryx
bazel run //:buildifier_lint
```

The CI will check for the correct formatting and linting.
See the `BUILD.bazel` file
in iceoryx workspace for available commands.
