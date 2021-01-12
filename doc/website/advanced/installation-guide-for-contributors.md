# Installation guide for contributors

## Build and run tests

While developing on iceoryx you want to know if your changes are breaking existing functions or if your newly written googletests are passing.
For that purpose we are generating cmake targets which are executing the tests. But first we need to build them:
```
cmake -Bbuild -Hiceoryx_meta -DBUILD_TEST=ON
cmake --build build
```
Cmake is automatically installing googletest as dependency and build the tests against it. Please note that if you want to build tests for extensions like the C-Binding you need to enable that in the cmake build. To build all tests simply add `-DBUILD_ALL` to the cmake command

Now lets execute the all tests:
```
cd iceoryx/build
make all_tests
```
Some of the tests are timing critical and needs a stable environment. We call them timing tests and have them in a separate target available:
```
make timing_tests
```
In iceoryx we distinguish between different testlevels. The most important are: Moduletests and Integrationtests.
Moduletests are basically Unit-tests where the focus is on class level with black-box testing.
In Integrationtests are multiple classes within one component (e.g. iceoryx_posh) tested together.
The sourcecode of the tests is placed into the folder `test` within the different iceoryx components. You can find there at least a folder `moduletests` and sometimes ``integrationtests`.

when you now want to create a new test you can place the sourcefile directly into the right folder. Cmake will automatically detect the new file when doing a clean build and will add it to a executable. There is no need to add a gtest main function because we already provide it.
For every test level are executables created, for example `posh_moduletests`. They are placed into the corresponding build folder (e.g. `iceoryx/build/posh/test/posh_moduletests`).

If you want to execute only individual testcases then you can use these executables and a gtest filter. Let's assume you want to execute only the `ServiceDescription_test` from the posh_moduletests, then you can do the following:
```
./build/posh/test/posh_moduletests --gtest_filter="ServiceDescription_test*"
```

## Use Sanitizer Scan

Due to the fact that iceoryx works a lot with system memory it should be ensured that errors like memory leaks are not introduced.
To prevent these, we use the clang toolchain which offers several tools for scanning the codebase. One of them is the [Address-Sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) which checks for example on dangling pointers.

In iceoryx below sanitizers are enabled at the moment.
- [AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
AddressSanitizer is a fast memory error detector. 
**NOTE :** AddressSanitizer exits on the first detected error, which means there could be more errors in the codebase when this error is reported.
- [LeakSanitizer](https://clang.llvm.org/docs/LeakSanitizer.html)
LeakSanitizer is a run-time memory leak detector. In iceoryx , it runs as part of the AdderssSanitizer.
- [UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
UndefinedBehaviorSanitizer (UBSan) is a fast undefined behavior detector. Iceoryx uses default behaviour ie `print a verbose error report and continue execution`

In iceoryx are scripts available to do the scan on your own. Additionally the Scans are running on the CI in every Pull-Request.
As Requirement you should install the clang compiler:
```
sudo apt install clang
```

Then you need to compile the iceoryx with the sanitizer flags:
```
./tools/iceoryx_build_test.sh build-strict build-all sanitize clang clean
```
After that we can run the tests with enabled sanitizer options:
```
cd build
../tools/run_all_tests.sh
```
When the tests are running without errors then it is fine, else an error report is shown with a stacktrace to find the place where the leak occurs. If the leak comes from an external dependency or shall be handled later then it is possible to set a function on a suppression list.
This should be only rarely used and only in coordination with an iceoryx maintainer.

## Iceoryx library build

Iceoryx consists of several libraries which have dependencies to each other. The goal is to have self-encapsulated library packages available
where the end-user can easily find it with the cmake command `find-package(...)`.
In the default case the iceoryx libraries are installed by `make install` into `/usr/lib` which need root access. To avoid that cmake gives you the possibility to install the libs into a custom folder.
This can be done by setting `-DCMAKE_INSTALL_PREFIX=/custom/install/path` as build-flag for the CMake file in iceoryx_meta.

Iceoryx_meta is a Cmake file which collects all libraries (utils, posh etc.) and extensions (binding_c, dds) together to have a single point for building. 
The alternate solution is provided for Ubuntu-users by having a build script `iceoryx_build_test.sh` in the tools folder.

Per default iceoryx is build as shared libraries because it is a cleaner solution for resolving dependency issues and it reduces the linker time while building.
This is done by the flag `BUILD_SHARED_LIBS` which is set to ON per default. If you want to have static libraries, just pass `-DBUILD_SHARED_LIBS=OFF` to Cmake or use `build-static` as flag in the build script.

If you want to share the iceoryx to other users, you can also create a debian package. You can create it by calling: `./tools/iceoryx_build_test.sh package` where it will be build it in `build_package`.
