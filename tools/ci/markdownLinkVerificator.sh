#!/usr/bin/env bash

ICEORYX_ROOT_PATH=$(git rev-parse --show-toplevel)

setupTerminalColors()
{
    if [[ -t 1 ]]
    then
        COLOR_BLACK="\e[30m"
        COLOR_RED="\e[31m"
        COLOR_GREEN="\e[32m"
        COLOR_YELLOW="\e[33m"
        COLOR_BLUE="\e[34m"
        COLOR_MAGENTA="\e[35m"
        COLOR_CYAN="\e[36m"
        COLOR_LIGHT_GRAY="\e[37m"
        COLOR_GRAY="\e[90m"
        COLOR_LIGHT_RED="\e[91m"
        COLOR_LIGHT_GREEN="\e[92m"
        COLOR_LIGHT_YELLOW="\e[93m"
        COLOR_LIGHT_BLUE="\e[94m"
        COLOR_LIGHT_MAGENTA="\e[95m"
        COLOR_LIGHT_MAGENTA="\e[96m"
        COLOR_WHITE="\e[97m"
        COLOR_RESET="\e[0m"
    else
        COLOR_BLACK=""
        COLOR_RED=""
        COLOR_GREEN=""
        COLOR_YELLOW=""
        COLOR_BLUE=""
        COLOR_MAGENTA=""
        COLOR_CYAN=""
        COLOR_LIGHT_GRAY=""
        COLOR_GRAY=""
        COLOR_LIGHT_RED=""
        COLOR_LIGHT_GREEN=""
        COLOR_LIGHT_YELLOW=""
        COLOR_LIGHT_BLUE=""
        COLOR_LIGHT_MAGENTA=""
        COLOR_LIGHT_MAGENTA=""
        COLOR_WHITE=""
        COLOR_RESET=""
    fi
}

setupTerminalFormat()
{
    if [[ -t 1 ]]
    then
        STATUS_MSG_SPACING="          "
        STATUS_MSG_POSITION="\r"
    else
        STATUS_MSG_SPACING=""
        STATUS_MSG_POSITION=""
    fi
}

doesWebURLExist()
{
    if curl --head --silent --fail $1 2> /dev/null 1>/dev/null ;
    then
        echo 1
    else
        echo 0
    fi
}

isWebLink()
{
    local RESULT
    RESULT=${RESULT}$(echo $1 | sed -n "s/^https:\/\/.*//p" | wc -l)
    RESULT=${RESULT}$(echo $1 | sed -n "s/^http:\/\/.*//p" | wc -l)
    echo $RESULT | grep 1 | wc -l
}

setupTerminalColors
setupTerminalFormat

for file in $(find $ICEORYX_ROOT_PATH -type f -iname "*.md" | grep -v ${ICEORYX_ROOT_PATH}/build | grep -v ${ICEORYX_ROOT_PATH}/.github)
do
    FILE_DIRECTORY=$(dirname $file)
    for link in $(cat $file | sed -n "s/.*\[\(.*\)](\([^)]*\)).*/\1[\2/p" | tr ' ' _)
    do
        LINK_NAME=$(echo $link | cut -f 1 -d '[')
        LINK_VALUE=$(echo $link | cut -f 2 -d '[')

        echo -en "  ${STATUS_MSG_SPACING}${COLOR_RESET}[ ${COLOR_LIGHT_BLUE}$LINK_NAME${COLOR_RESET} ] "
        if [[ $(isWebLink $LINK_VALUE) == "1" ]]
        then
            echo -en "Verify WebLink : ${COLOR_LIGHT_BLUE}$LINK_VALUE${COLOR_RESET} "
            if [[ $(doesWebURLExist $LINK_VALUE) == "1" ]]
            then
                echo -e "${STATUS_MSG_POSITION}<${COLOR_LIGHT_GREEN} success${COLOR_RESET} >"
            else
                echo -e "${STATUS_MSG_POSITION}<${COLOR_LIGHT_RED} failed${COLOR_RESET} >"
            fi
        else
            echo bla
        fi
    done
done
