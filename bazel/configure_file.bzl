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

The configure_file rule imitates the similar CMake function for template expansion: https://cmake.org/cmake/help/latest/command/configure_file.html
"""

def _configure_file_impl(ctx):
    ctx.actions.expand_template(
        template = ctx.file.src,
        output = ctx.outputs.out,
        substitutions = {
            "@" + k + "@": v
            for k, v in ctx.attr.config.items()
        },
    )
    files = depset(direct = [ctx.outputs.out])
    runfiles = ctx.runfiles(files = [ctx.outputs.out])
    return [DefaultInfo(files = files, runfiles = runfiles)]

configure_file = rule(
    implementation = _configure_file_impl,
    provides = [DefaultInfo],
    attrs = {
        "config": attr.string_dict(mandatory = True),
        "out": attr.output(mandatory = True),
        "src": attr.label(mandatory = True, allow_single_file = True),
    },
)
