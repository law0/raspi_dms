#!/bin/bash

LOCAL_PWD=$(pwd)

mkdir -p src && cd src
########## Download opencv
wget -O opencv.tar.gz https://github.com/opencv/opencv/archive/refs/tags/4.5.2.tar.gz
tar xf opencv.tar.gz
wget -O opencv_contrib.tar.gz https://github.com/opencv/opencv_contrib/archive/refs/tags/4.5.2.tar.gz
tar xf opencv_contrib.tar.gz
rm *.tar.gz

cd $LOCAL_PWD

