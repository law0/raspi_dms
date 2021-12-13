#!/bin/bash

set -ex

LOCAL_PWD=$(pwd)

if [ -z "$1" ]
then
    docker build -t rasp-cross-compile --build-arg local_pwd=${LOCAL_PWD} .
fi

CAMERA_OPTIONS=""
if [ -f "/dev/video0" ]
then
    for device in "$(ls /dev/video*)"
    do
        CAMERA_OPTIONS="${CAMERA_OPTIONS} --device ${device}"
    done
fi

docker run --privileged --user $(id -u):$(id -g) \
    --rm \
    -v $(pwd):$(pwd) \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -e DISPLAY=$DISPLAY \
    ${CAMERA_OPTIONS} \
    -w $(pwd) \
    -it \
    rasp-cross-compile bash
