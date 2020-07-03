#!/bin/bash

build_iceoryx_docker() {
	if [[ "$(docker images -q iceoryx:latest 2> /dev/null)" == "" ]]; then
		echo "Building base iceoryx docker image."
		../../tools/docker/build_iceoryx_docker.sh
	fi
}

echo "Select an example:"
echo ""
options=("Simple Internode-Communcation" "Quit")
select opt in "${options[@]}"
do
    case $opt in
        "Simple Internode-Communcation")
			build_iceoryx_docker
			cd docker
			docker-compose -f simple_internode_communication.yml up
            break
            ;;
        "Quit")
            break
            ;;
        *) echo invalid option;;

    esac
done
