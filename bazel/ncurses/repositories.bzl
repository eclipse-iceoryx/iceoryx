# Copyright (c) 2024 by Fernride GmbH. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

"""This module prepares the ncurses dependency."""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:utils.bzl", "maybe")

def load_ncurses_repositories():
    maybe(
        http_archive,
        name = "rules_foreign_cc",
        sha256 = "a2e6fb56e649c1ee79703e99aa0c9d13c6cc53c8d7a0cbb8797ab2888bbc99a3",
        strip_prefix = "rules_foreign_cc-0.12.0",
        url = "https://github.com/bazelbuild/rules_foreign_cc/releases/download/0.12.0/rules_foreign_cc-0.12.0.tar.gz",
    )
    maybe(
        http_archive,
        name = "ncurses",
        url = "https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.3.tar.gz",
        sha256 = "97fc51ac2b085d4cde31ef4d2c3122c21abc217e9090a43a30fc5ec21684e059",
        strip_prefix = "ncurses-6.3",
        build_file = "//bazel/ncurses:ncurses.BUILD",
    )
