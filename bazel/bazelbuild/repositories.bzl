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

Loads dependent repositories from https://github.com/bazelbuild
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

BAZELBUILD_RULES_CC_VERSION = "0.0.1"

def load_com_github_bazelbuild_rules_cc_repositories():
    maybe(
        name = "bazelbuild_rules_cc",
        repo_rule = http_archive,
        sha256 = "4dccbfd22c0def164c8f47458bd50e0c7148f3d92002cdb459c2a96a68498241",
        urls = [
            "https://github.com/bazelbuild/rules_cc/releases/download/{version}/rules_cc-{version}.tar.gz".format(version = BAZELBUILD_RULES_CC_VERSION),
        ],
    )

COM_GITHUB_BAZELBUILD_BUILDTOOLS_VERSION = "5.1.0"

def load_com_github_bazelbuild_buildtools_repositories():
    maybe(
        name = "com_github_bazelbuild_buildtools",
        repo_rule = http_archive,
        sha256 = "e3bb0dc8b0274ea1aca75f1f8c0c835adbe589708ea89bf698069d0790701ea3",
        strip_prefix = "buildtools-{version}".format(version = COM_GITHUB_BAZELBUILD_BUILDTOOLS_VERSION),
        urls = [
            "https://github.com/bazelbuild/buildtools/archive/refs/tags/{version}.tar.gz".format(version = COM_GITHUB_BAZELBUILD_BUILDTOOLS_VERSION),
        ],
    )

IO_BAZEL_RULES_GO_VERSION = "0.45.1"

def load_io_bazel_rules_go_repositories():
    maybe(
        name = "io_bazel_rules_go",
        repo_rule = http_archive,
        sha256 = "6734a719993b1ba4ebe9806e853864395a8d3968ad27f9dd759c196b3eb3abe8",
        urls = [
            "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v{version}/rules_go-v{version}.zip".format(version = IO_BAZEL_RULES_GO_VERSION),
            "https://github.com/bazelbuild/rules_go/releases/download/v{version}/rules_go-v{version}.zip".format(version = IO_BAZEL_RULES_GO_VERSION),
        ],
    )

COM_GOOGLE_PROTOBUF_VERSION = "3.13.0"

def load_com_google_protobuf_repositories():
    maybe(
        name = "com_google_protobuf",
        repo_rule = http_archive,
        sha256 = "9b4ee22c250fe31b16f1a24d61467e40780a3fbb9b91c3b65be2a376ed913a1a",
        strip_prefix = "protobuf-{version}".format(version = COM_GOOGLE_PROTOBUF_VERSION),
        urls = [
            "https://github.com/protocolbuffers/protobuf/archive/v{version}.tar.gz".format(version = COM_GOOGLE_PROTOBUF_VERSION),
        ],
    )
