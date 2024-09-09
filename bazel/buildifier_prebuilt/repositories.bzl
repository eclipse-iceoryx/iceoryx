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

This module downloads the buildifier prebuilt project and its deps: https://github.com/keith/buildifier-prebuilt
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

BAZEL_SKYLIB_VERSION = "1.7.1"

def load_bazel_skylib_repositories():
    maybe(
        name = "bazel_skylib",
        repo_rule = http_archive,
        sha256 = "bc283cdfcd526a52c3201279cda4bc298652efa898b10b4db0837dc51652756f",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = BAZEL_SKYLIB_VERSION),
            "https://github.com/bazelbuild/bazel-skylib/releases/download/{version}/bazel-skylib-{version}.tar.gz".format(version = BAZEL_SKYLIB_VERSION),
        ],
    )

BUILDIFIER_PREBUILT_VERSION = "7.3.1"

def load_buildifier_prebuilt_repositories():
    maybe(
        name = "buildifier_prebuilt",
        repo_rule = http_archive,
        sha256 = "7f85b688a4b558e2d9099340cfb510ba7179f829454fba842370bccffb67d6cc",
        strip_prefix = "buildifier-prebuilt-{version}".format(version = BUILDIFIER_PREBUILT_VERSION),
        urls = [
            "http://github.com/keith/buildifier-prebuilt/archive/{version}.tar.gz".format(version = BUILDIFIER_PREBUILT_VERSION),
        ],
    )
