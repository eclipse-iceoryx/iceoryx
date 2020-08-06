# Contributing to Eclipse iceoryx

Thanks for your interest in this project.

## Project description

In domains like automotive, robotics or gaming, a huge amount of data must be
transferred between different parts of the system. If these parts are actually
different processes on a POSIX based operating system like Linux, this huge
amount of data has to be transferred via an inter-process-communication (IPC)
mechanism.

* https://projects.eclipse.org/projects/technology.iceoryx

## Developer resources

Information regarding source code management, builds, coding standards, and
more.

* https://projects.eclipse.org/projects/technology.iceoryx/developer

The project maintains the following source code repositories

* https://github.com/eclipse/iceoryx

## Eclipse Contributor Agreement

Before your contribution can be accepted by the project team, contributors must
electronically sign the Eclipse Contributor Agreement (ECA).

* http://www.eclipse.org/legal/ECA.php

Commits that are provided by non-committers must have a Signed-off-by field in
the footer indicating that the author is aware of the terms by which the
contribution has been provided to the project. The non-committer must
additionally have an Eclipse Foundation account and must have a signed Eclipse
Contributor Agreement (ECA) on file.

For more information, please see the Eclipse Committer Handbook:
https://www.eclipse.org/projects/handbook/#resources-commit

## Contact

Contact the project developers via the project's "dev" list.

* iceoryx-dev@eclipse.org

## Feature request and bugs

We love pull requests! The next sections try to cover most of the relevant questions. For larger contributions or
architectural changes, we'd kindly ask you to get in touch with one of the maintainers beforehand. If you would like to
report a bug or propose a new feature, please raise an issue before raising a pull request. This makes it easier to
track. Please make sure you have:

1. Signed the [ECA](http://www.eclipse.org/legal/ECA.php)
2. All commits have been commited with `git commit -s`
3. You open your pull request towards the base branch `staging`

## Coding style

We love the [C++ core guidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). If in doubt please try
to follow them as well as our unwritten conventions in the existing parts of the code base.
Please format your code with the provided [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and
[clang-tidy](https://clang.llvm.org/extra/clang-tidy/) before raising a pull request. Lots of IDEs do read the
clang-format file these days.

We created some convenient rules to highlight some bits that you might not be used to in other FOSS projects. They are
helpful to build embedded systems for safety fields like automotive or avionics. It is possible that not the whole
codebase follows these rules, things are work in progress.

1) **No heap is allowed**, static memory management hugely decreases the complexity of your software (e.g. cxx::vector
    without heap)
2) **No exception are allowed**, all function and methods need to have `noexcept` in their signature
3) **No undefined behavior**, zero-cost abstract is not feasible in high safety environments
4) **Use C++11**, however we try to introduce C++14 as fast as possible
5) **[Rule of Five](https://en.wikipedia.org/wiki/Rule_of_three_(C%2B%2B_programming))**, if there is a non-default
    destructor needed, the rule of five has to be applied
6) **[STL](https://en.wikipedia.org/wiki/Standard_Template_Library)**, we aim to be compatible towards the STL, but
    our code may contain additions which are not compatible with the STL (e.g. `iox::cxx::vector::emplace_back()`
    does return a bool)
7) **Always use `iox::log::Logger`**, instead of `printf()`
8) **Always use `iox::ErrorHandler()`**, instead of the direct STL calls

### Naming conventions

* File names with `lower_snake_case`: `my_thing.hpp`
* Structs, classes and enum classes in `UpperCamelCase`: `class MyClass{}`
* Methods and variables in `lowerCamelCase`: `uint16_t myVariable`
* Compile time constants, also enum values in `UPPER_SNAKE_CASE`: `static constexpr uint16_t MY_CONSTANT`
* Class members start with `m_`: `m_myMember`
* Namespaces in `lower_snake_case` : `my_namespace`

### Doxygen

Please use [doxygen](http://www.doxygen.nl/) to document your code.

The following doxygen comments are required for public API headers:

    /// @brief short description
    /// @param[in] / [out] / [in,out] name description
    /// @return description

## Folder structure

The folder structure boils down to:

* iceoryx_COMPONENT
  * cmake: All cmakes files go here, needed for `find_pkg()`
  * doc: Manuals and documentation
  * include: public headers with stable API
    * internal: public headers with unstable API, which might change quite frequently
  * source: implementation files
  * test: unit and integrations tests
  * CMakeLists.txt: Build the component separately
* examples_iceoryx: Examples described in the main [Readme.md](./README.md#user-content-examples)

All new code should follow the folder structure.

## Testing

We use [Google test](https://github.com/google/googletest) for our unit and integration tests. We require compatibility
with the version 1.8.1.

### Unit tests (aka module tests)

Unit tests are black box tests that test the public interface of a class. They are required for all new code.

### Integration tests

Integration tests are composition of more than one class and test their interaction. They are optional for new code.

## Legal & Compliance

### Dependencies

* [POSIX](https://en.wikipedia.org/wiki/POSIX)
Iceoryx aims to be fully POSIX-compliant towards the current revision POSIX.1-2017 (IEEE 1003.1-2017). Please write
your code as portable as possible. Currently our focus is [QNX](https://blackberry.qnx.com/en) (QCC 5.4) and Linux (GCC 7.5.0).

* [ACL](https://en.wikipedia.org/wiki/Access-control_list)

* [ncurses](https://www.gnu.org/software/ncurses/)

### Safety & security

We aim for [ASIL-D](https://en.wikipedia.org/wiki/Automotive_Safety_Integrity_Level#ASIL_D) compliance. The
[ISO26262](https://en.wikipedia.org/wiki/ISO_26262) is also a good read-up if you want to learn more about automotive
safety. A nice introduction [video](https://www.youtube.com/watch?v=F4GzsA00s5I) was presented on CppCon 2019.

We have a [partnership](https://www.perforce.com/blog/qac/why-eclipse-iceoryx-uses-helix-qac) with [Perforce](https://www.perforce.com) and use
[Helix QAC++ 2019.2](https://www.perforce.com/products/helix-qac) to perform a static-code analysis.

Github [labels](https://github.com/eclipse/iceoryx/labels) are used to group issues into the rulesets:

| Ruleset name | Github issue label |
|---|---|
| [MISRA](https://www.misra.org.uk/) C++ 2008 | MISRA |
| [Adaptive AUTOSAR](https://www.autosar.org/fileadmin/user_upload/standards/adaptive/17-03/AUTOSAR_RS_CPP14Guidelines.pdf) C++14 | AUTOSAR |
| [SEI CERT C++](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682) 2016 Coding Standard | CERT |

If one of the rules is not followed, a rationale is added in the following manner:

With a comment in the same line:
    *mynullptr = foo; // PRQA S 4242 # Short description why

With a comment one line above (with the number after the warning number, next ’n’ lines are inclusive)
    // PRQA S 4242 1 # Short description why
    *mynullptr = foo;

Don't be afraid if you don't have Helix QAC++ available. As we want to make it easy for developers to contribute,
please use the ``staging`` branch and we'll run the QAC++ scan and get back to you.

Results will be available on this [Helix QAC dashboard](https://qaverify.programmingresearch.com/). Please contact one
of the maintainers, if you're interested in getting access.

It is possible that not the whole codebase follows these rules, things are work in progress. But this is where we want
go. As of now we don't have any continous integration checks implemented but will rely on reviews during the pull
requests. We're planning to introduce continous integration checks in the near future.

### Header

Each source file needs to have this header:

    // Copyright (c) [year] by [Name of author]. All rights reserved.
    //
    // Licensed under the Apache License, Version 2.0 (the "License");
    // you may not use this file except in compliance with the License.
    // You may obtain a copy of the License at
    //
    //     http://www.apache.org/licenses/LICENSE-2.0
    //
    // Unless required by applicable law or agreed to in writing, software
    // distributed under the License is distributed on an "AS IS" BASIS,
    // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    // See the License for the specific language governing permissions and
    // limitations under the License.

## Quality levels

CMake targets can be developed according to different quality levels. Despite developing some of our targets according
to automotive standards like ISO26262, the code base standalone does NOT legitimize the usage in a safety critical
system. All requirements of a lower quality level are included in higher quality levels e.g. quality level 4 is
included in quality level 3.

Also see [ROS quality levels](https://github.com/ros-infrastructure/rep/blob/master/rep-2004.rst).

### Quality level 5

This quality level is the default quality level. It is meant for examples and helper tools.

* Reviewed by two approver
* License and copyright statement available
* No version policy required
* No unit tests required

### Quality level 4

This quality level is meant for all targets that need tier 1 support in ROS2.

* Basic unit tests are available

### Quality level 3

* No compiler warnings
* Doxygen and documentation available
* Test specification available
* Version policy required
* Level 8 and 9 warnings in Helix QAC addressed

### Quality level 2

* Unit tests have full statement and branch coverage

### Quality level 1

* Warnings in Helix QAC addressed
* Code coverage according to [MC/DC](https://en.wikipedia.org/wiki/Modified_condition/decision_coverage) available
