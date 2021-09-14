#!/bin/bash

#sudo apt install python3-dev python3-numpy python-dev python-numpy libgtk-3-dev libcanberra-gtk3-dev libtiff-dev zlib1g-dev libjpeg-dev libpng-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libxvidcore-dev libx264-dev gfortran libopenblas-dev liblapack-dev
export TOOLCHAIN_FILE_OVERRIDE=LOCAL
./build_libs.sh -j4 && ./build.sh -j4
