#!/usr/bin/env python3

import os
import requests
import sys
import time


def main():
    data = {
        'token': os.environ['AXIVION_TRIGGER_TOKEN'],
        'ref': os.environ.get('AXIVION_REF_NAME', 'master'),
    }
    commit_sha = os.environ['GITHUB_SHA']
    r = requests.post(
        f'https://gitlab.com/api/v4/projects/24081973/trigger/pipeline?variables[ICEORYX_SHA]={commit_sha}',
        json=data)

    if r.status_code != 201:
        print(f'ERROR: Pipeline trigger failed: {r.status_code}', file=sys.stderr)
        sys.exit(1)

    pipeline_id = r.json()['id']
    print(pipeline_id)


if __name__ == "__main__":
     main()
