#!/bin/bash
#
# Runs the iceoryx docker.
# The docker will immediately start RouDi. It can then be interacted with by
# either binding a shell to it or connecting via screen.
#
# NOTE: The container must first be built. This can be done using build_and_run.sh
#

print_usage () {
        echo "Usage: $0 <shm-size>"
}

SHM_SIZE=$1
if [ -z $SHM_SIZE];
then
	SHM_SIZE=700M
fi

docker run -it --rm --shm-size $SHM_SIZE --name=iceoryx-roudi iceoryx:latest
