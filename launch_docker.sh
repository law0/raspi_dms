#!/bin/bash

docker build -t rasp-cross-compile .
docker run --privileged --rm -v $(pwd):$(pwd) -w $(pwd) -it rasp-cross-compile bash
