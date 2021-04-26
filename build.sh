#!/bin/bash

LOCAL_PWD=$(pwd)

echo $LOCAL_PWD

mkdir -p out/staging
mkdir -p out/final
mkdir -p out/build

SRC=${LOCAL_PWD}/src
STAGING=${LOCAL_PWD}/out/staging
FINAL=${LOCAL_PWD}/out/final
BUILD=${LOCAL_PWD}/out/build

TOOLCHAIN_FILE=${SRC}/opencv-4.5.2/platforms/linux/arm-gnueabi.toolchain.cmake

##############################################
### Build opencv
##############################################
mkdir -p ${FINAL}/opencv-4.5.2
mkdir -p ${BUILD}/opencv && cd ${BUILD}/opencv
cmake -D CMAKE_BUILD_TYPE=RELEASE \
  -D CMAKE_INSTALL_PREFIX=${FINAL}/opencv-4.5.2 \
  -D CMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
  -D OPENCV_EXTRA_MODULES_PATH=${SRC}/opencv_contrib-4.5.2/modules \
  -D OPENCV_ENABLE_NONFREE=ON \
  -D ENABLE_NEON=ON \
  -D ENABLE_VFPV3=ON \
  -D BUILD_TESTS=OFF \
  -D BUILD_DOCS=OFF \
  -D PYTHON2_INCLUDE_PATH=/usr/include/python2.7 \
  -D PYTHON2_LIBRARIES=/usr/lib/arm-linux-gnueabihf/libpython2.7.so \
  -D PYTHON2_NUMPY_INCLUDE_DIRS=/usr/lib/python2/dist-packages/numpy/core/include \
  -D PYTHON3_INCLUDE_PATH=/usr/include/python3.6m \
  -D PYTHON3_LIBRARIES=/usr/lib/arm-linux-gnueabihf/libpython3.6m.so \
  -D PYTHON3_NUMPY_INCLUDE_DIRS=/usr/lib/python3/dist-packages/numpy/core/include \
  -D BUILD_OPENCV_PYTHON2=ON \
  -D BUILD_OPENCV_PYTHON3=ON \
  -D BUILD_EXAMPLES=ON \
   ${SRC}/opencv-4.5.2

make $@
make install

#Wrong naming of python lib
cd ${FINAL}/opencv-4.5.2/lib/python3.6/dist-packages/cv2/python-3.6/
cp cv2.cpython-36m-x86_64-linux-gnu.so cv2.so

cd $LOCAL_PWD
tar -cjvf ${FINAL}/opencv-4.5.2-armhf.tar.bz2 ${FINAL}/opencv-4.5.2

cd $LOCAL_PWD
