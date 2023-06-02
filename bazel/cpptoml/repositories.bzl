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

This module downloads the cpptoml project: https://github.com/skystrife/cpptoml
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

CPPTOML_VERSION = "0.1.1"

def load_cpptoml_repositories():
    maybe(
        name = "cpptoml",
        repo_rule = http_archive,
        sha256 = "23af72468cfd4040984d46a0dd2a609538579c78ddc429d6b8fd7a10a6e24403",
        url = "https://github.com/skystrife/cpptoml/archive/refs/tags/v{version}.tar.gz".format(version = CPPTOML_VERSION),
        strip_prefix = "cpptoml-{version}".format(version = CPPTOML_VERSION),
        build_file = Label("//bazel/cpptoml:cpptoml.BUILD"),
    )
