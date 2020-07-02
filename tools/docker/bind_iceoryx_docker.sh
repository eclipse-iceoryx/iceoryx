#!/bin/bash
#
# Binds a shell to a running iceoryx docker container.
#

print_usage () {
        echo "Usage: $0"
}

CONTAINER_NAME=iceoryx-roudi

if [ ! $(docker inspect -f '{{.State.Running}}' $CONTAINER_NAME 2> /dev/null) ]
then
    echo "Iceoryx docker container not running. Did you forget to start it ?"
    exit
fi

echo "Binding to container: $CONTAINER_NAME"

docker exec -it $CONTAINER_NAME /bin/bash