#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
ORIGIN_DIR=$(pwd)

cd $SCRIPT_DIR
OPENCV_SAMPLES_DATA_PATH_HINT=/opt/opencv-4.5.2/share/opencv4 \
  LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/opencv-4.5.2/lib \
  ./raspidms $@
cd $ORIGIN_DIR

