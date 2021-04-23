#!/bin/bash

docker build -t rasp-cross-compile .
docker run --privileged --user $(id -u):$(id -g) --rm -v $(pwd):$(pwd) -w $(pwd) -it rasp-cross-compile bash
