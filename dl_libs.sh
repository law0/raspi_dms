#!/bin/bash

LOCAL_PWD=$(pwd)

mkdir -p src && cd src
########## Download opencv
wget -O opencv.tar.gz https://github.com/opencv/opencv/archive/refs/tags/4.5.2.tar.gz
tar xf opencv.tar.gz
wget -O opencv_contrib.tar.gz https://github.com/opencv/opencv_contrib/archive/refs/tags/4.5.2.tar.gz
tar xf opencv_contrib.tar.gz
rm *.tar.gz

########## Download dlib
git clone --depth 1 --branch v19.22 https://github.com/davisking/dlib.git

########## Download tensorflow
git clone --depth 1 --branch v2.6.2 https://github.com/tensorflow/tensorflow.git
wget https://storage.googleapis.com/mirror.tensorflow.org/developer.arm.com/media/Files/downloads/gnu-a/8.3-2019.03/binrel/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz \
-O gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz
mkdir -p toolchain
tar xvf gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz -C toolchain
rm gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf.tar.xz

cd $LOCAL_PWD

