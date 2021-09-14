#!/bin/bash

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../../opencv-4.5.2/lib ./raspidms $@
