#!/usr/bin/env bash
# Copyright (c) 2019 by Cornel Izbasa. All rights reserved.
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

set -euo pipefail

REVISION="current"
IMAGE_NAME="iceoryx:${REVISION}"
CONTAINER_NAME=roudi
ICEORYX_BUILD=/iceoryx/build
DOCKERFILE_DIR="$(dirname "$(readlink -fm "$0")")"
ICEORYX_SRC="$(dirname "$(dirname "${DOCKERFILE_DIR}")")"
LAUNCH_FLAGS=(\
	-it \
	--rm \
	--shm-size 700M \
)

cd "${ICEORYX_SRC}"
docker build -f "${DOCKERFILE_DIR}/Dockerfile" --build-arg B_ICEORYX_BUILD="${ICEORYX_BUILD}" --build-arg REVISION="${REVISION}" . -t "${IMAGE_NAME}"
docker run "${LAUNCH_FLAGS[@]}" --name="${CONTAINER_NAME}" "${IMAGE_NAME}"
