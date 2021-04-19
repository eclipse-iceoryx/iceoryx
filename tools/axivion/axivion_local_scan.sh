#!/bin/bash
set -e

ICEORYX_DIR=$(git rev-parse --show-toplevel)
export ICEORYX_DIR
HOST_HOSTNAME="localhost"

source /opt/axivion_bauhaus/bauhaus-kshrc
confdir="$(dirname "$(readlink -f "$ICEORYX_DIR")")"
confdir="$confdir"/iceoryx/tools/axivion
export BAUHAUS_CONFIG="$confdir"
export NUM_PROCESSES=$(nproc)

if [[ ! -d "$ICEORYX_DIR/axivion_results" ]]; then
    mkdir -p $ICEORYX_DIR/axivion_results
else
    export AXIVION_LOCAL_BUILD=1
fi

export AXIVION_DATABASES_DIR="$(realpath "$ICEORYX_DIR/axivion_results")"
export AXIVION_DASHBOARD_URL="http://localhost/axivion"


gccsetup --gcc gcc --g++ g++ "$confdir"
bauhaus_config \
--add /cafeCC/Frontend/C/Compiler/Options/std_position_format=--std_position_format \
--add /cafeCC/Frontend/C++/Compiler/Options/std_position_format=--std_position_format \
--add /cafeCC/Frontend/Linker/Options/jott=-j \
--add /cafeCC/Frontend/Linker/Options/kleinI=-i2 \
--add /cafeCC/Frontend/Linker/Options/sg=--separate_groups \
--add /cafeCC/Frontend/Linker/Options/grossI=-I2000 \
--add /cafeCC/Frontend/Linker/Options/unused=--include_unused \
"$confdir"

pushd $ICEORYX_DIR
export AXIVION_VERSION_NAME="$(git describe --always --abbrev=1 --tags  2>/dev/null)"
popd

axivion_ci -j --verbose
