"""
Copyright (c) 2022 by Apex.AI Inc. All rights reserved.

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
"""

load("//bazel/bazelbuild:repositories.bzl", "load_com_github_bazelbuild_rules_cc_repositories")
load("//bazel/buildifier_prebuilt:repositories.bzl", "load_bazel_skylib_repositories", "load_buildifier_prebuilt_repositories")
load("//bazel/cpptoml:repositories.bzl", "load_cpptoml_repositories")
load("//bazel/googletest:repositories.bzl", "load_googletest_repositories")

def load_repositories():
    """
    Loads repositories for iceoryx dependencies
    """
    load_com_github_bazelbuild_rules_cc_repositories()
    load_bazel_skylib_repositories()
    load_buildifier_prebuilt_repositories()
    load_googletest_repositories()
    load_cpptoml_repositories()
