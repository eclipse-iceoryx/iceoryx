#!/usr/bin/env python

# Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

import argparse
import subprocess
import os
from pathlib import Path


def main():

    repo_dir = subprocess.Popen(['git', 'rev-parse', '--show-toplevel'],
                                stdout=subprocess.PIPE).communicate()[0].rstrip().decode('utf-8')
    web_repo = "https://github.com/eclipse-iceoryx/iceoryx-web.git"
    iceoryx_components = ["utils", "posh"]
    web_path = Path(repo_dir, '..', 'iceoryx-web')
    mkdocs_path = os.path.join(repo_dir, 'mkdocs.yml')

    parser = argparse.ArgumentParser(
        description='Iceoryx website script.')
    parser.add_argument('--iceversion', '-j', nargs='?', const='', default='0',
                        help='Set version for iceoryx website')
    args = parser.parse_args()

    for cmp in iceoryx_components:
        tmp = os.path.join(repo_dir, 'iceoryx_' + cmp, 'doc')
        os.chdir(tmp)
        print("Current doc dir:", os.getcwd())
        doxygen_call = subprocess.run(
            ['doxygen', 'doxyfile-' + cmp], check=True)

        api_ref = os.path.join(repo_dir, 'doc', 'website', 'API-reference')
        Path(api_ref).mkdir(exist_ok=True)
        output_folder = os.path.join(api_ref, cmp)
        Path(output_folder).mkdir(exist_ok=True)

        input_folder = os.path.join(tmp, 'xml')
        doxybook_call = subprocess.run(
            ['doxybook2', '-i', input_folder, '-o', output_folder], check=True)

    # Generate HTML and push to GitHub pages
    if not web_path.exists():
        os.chdir(web_path)
        subprocess.Popen(['git', 'clone', web_repo])

    os.chdir('iceoryx-web')
    mkdocs_call = subprocess.run(
        ['mkdocs', 'gh-deploy', '--config-file', mkdocs_path, '--remote-branch', args.iceversion], check=True)


if __name__ == "__main__":
    main()
