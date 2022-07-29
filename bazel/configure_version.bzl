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

This module set compile-time variables and set them accordingly in generated files
"""

SCRIPT = """#!/bin/bash

set -e

version_file="{version_file}"
input_file="{input_file}"
output_file="{output_file}"

IFS="-" read version_number version_suffix <<< $(<"$version_file")
IFS="." read major_version minor_version patch_version tweak_version <<< $version_number
[ -z "$version_suffix" ] && version_delim="" || version_delim="-"

data="$(<"$input_file")"
data=${{data/@PROJECT_VERSION_MAJOR@/${{major_version:-0}}}}
data=${{data/@PROJECT_VERSION_MINOR@/${{minor_version:-0}}}}
data=${{data/@PROJECT_VERSION_PATCH@/${{patch_version:-0}}}}
data=${{data/@IOX_VERSION_TWEAK@/${{tweak_version:-0}}}}
data=${{data/@PROJECT_VERSION@/${{version_number}}}}
data=${{data/@IOX_VERSION_SUFFIX@/${{version_delim}}${{version_suffix}}}}
data=${{data/@ICEORYX_BUILDDATE@/builddate_not_set}}
data=${{data/@ICEORYX_SHA1@/sha1_not_set}}

echo "$data" > $output_file
"""

def _configure_version_impl(ctx):
    ctx.actions.run_shell(
        command = SCRIPT.format(
            version_file = ctx.files.version_from[0].path,
            input_file = ctx.files.src[0].path,
            output_file = ctx.outputs.out.path,
        ),
        inputs = ctx.files.src + ctx.files.version_from,
        outputs = [ctx.outputs.out],
    )
    files = depset(direct = [ctx.outputs.out])
    return [DefaultInfo(files = files)]

configure_version = rule(
    implementation = _configure_version_impl,
    provides = [DefaultInfo],
    attrs = {
        "out": attr.output(mandatory = True),
        "src": attr.label(mandatory = True, allow_single_file = True),
        "version_from": attr.label(mandatory = True, allow_single_file = True),
    },
)
