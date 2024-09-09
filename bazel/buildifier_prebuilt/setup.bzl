"""
Copyright 2024, Eclipse Foundation and the iceoryx contributors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

SPDX-License-Identifier: Apache-2.0

This module prepares the buildifier prebuilt project and its deps: https://github.com/keith/buildifier-prebuilt
"""

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
load("@buildifier_prebuilt//:defs.bzl", "buildifier_prebuilt_register_toolchains")
load("@buildifier_prebuilt//:deps.bzl", "buildifier_prebuilt_deps")

def setup_buildifier_prebuilt():
    buildifier_prebuilt_deps()

    bazel_skylib_workspace()

    buildifier_prebuilt_register_toolchains()
