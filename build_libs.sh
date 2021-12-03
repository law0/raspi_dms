#!/bin/bash

LOCAL_PWD=$(pwd)

echo $LOCAL_PWD

SRC=${LOCAL_PWD}/src
FINAL=${LOCAL_PWD}/out/final
PKGCONFIG=${FINAL}/pkgconfig
BUILD=${LOCAL_PWD}/out/build

mkdir -p $FINAL
mkdir -p $PKGCONFIG
mkdir -p $BUILD

CROSS_BUILD_OPTIONS=""
TOOLCHAIN_FILE=${SRC}/opencv-4.5.2/platforms/linux/arm-gnueabi.toolchain.cmake
#PYTHON_OPTIONS="-D PYTHON2_INCLUDE_PATH=/usr/include/python2.7 \
#  -D PYTHON2_LIBRARIES=/usr/lib/arm-linux-gnueabihf/libpython2.7.so \
#  -D PYTHON2_NUMPY_INCLUDE_DIRS=/usr/lib/python2/dist-packages/numpy/core/include \
#  -D PYTHON3_INCLUDE_PATH=/usr/include/python3.6m \
#  -D PYTHON3_LIBRARIES=/usr/lib/arm-linux-gnueabihf/libpython3.6m.so \
#  -D PYTHON3_NUMPY_INCLUDE_DIRS=/usr/lib/python3/dist-packages/numpy/core/include "


#PYTHON_OPTIONS="-D PYTHON3_INCLUDE_PATH=/usr/include/python3.6m \
#  -D PYTHON3_LIBRARIES=/usr/lib/arm-linux-gnueabihf/libpython3.6m.so \
#"  -D PYTHON3_NUMPY_INCLUDE_DIRS=/usr/lib/python3/dist-packages/numpy/core/include"

if [ -n "$TOOLCHAIN_FILE_OVERRIDE" ]
then
    if [ "$TOOLCHAIN_FILE_OVERRIDE" != "LOCAL" ]
    then
        TOOLCHAIN_FILE=$TOOLCHAIN_FILE_OVERRIDE
    
    else #LOCAL BUILD
        TOOLCHAIN_FILE=""
        PYTHON_OPTIONS="-D PYTHON2_NUMPY_INCLUDE_DIRS=/usr/lib/python2/dist-packages/numpy/core/include \
                        -D PYTHON3_NUMPY_INCLUDE_DIRS=/usr/lib/python3/dist-packages/numpy/core/include "
    fi
fi

if [ -n "$TOOLCHAIN_FILE" ]
then

    OPENCV_CROSS_BUILD_OPTIONS="-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
    -D ENABLE_NEON=ON \
    -D ENABLE_VFPV3=ON "

    ARMCC_FLAGS="-march=armv7-a -mfpu=neon-vfpv4 -funsafe-math-optimizations"
    ARMCC_PREFIX="${SRC}/tflite_toolchain/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-"

    TFLITE_CROSS_BUILD_OPTIONS="-DCMAKE_SYSTEM_NAME=Linux \
   -DCMAKE_C_COMPILER=${ARMCC_PREFIX}gcc \
   -DCMAKE_CXX_COMPILER=${ARMCC_PREFIX}g++ \
   -DCMAKE_SYSTEM_PROCESSOR=armv7 \
   -DCMAKE_C_FLAGS=${ARMCC_FLAGS} \
   -DCMAKE_CC_FLAGS=${ARMCC_FLAGS} "

fi

##############################################
### Build opencv
##############################################
mkdir -p ${BUILD}/opencv && cd ${BUILD}/opencv
mkdir -p ${FINAL}/opencv-4.5.2

cmake -D CMAKE_BUILD_TYPE=RELEASE \
  -D CMAKE_INSTALL_PREFIX=${FINAL}/opencv-4.5.2 \
   ${OPENCV_CROSS_BUILD_OPTIONS} \
  -D OPENCV_EXTRA_MODULES_PATH=${SRC}/opencv_contrib-4.5.2/modules \
  -D OPENCV_ENABLE_NONFREE=ON \
  -D BUILD_TESTS=OFF \
  -D BUILD_DOCS=OFF \
  -D BUILD_EXAMPLES=ON \
   ${SRC}/opencv-4.5.2

make $@
make install

# Wrong naming of python lib
cd ${FINAL}/opencv-4.5.2/lib/python3.6/dist-packages/cv2/python-3.6/
cp cv2.cpython-36m-x86_64-linux-gnu.so cv2.so

# Pkgconfig
cd $LOCAL_PWD
echo "libdir = ${FINAL}/opencv-4.5.2/lib" > ${PKGCONFIG}/opencv.pc
echo "includedir = ${FINAL}/opencv-4.5.2/include/opencv4" >> ${PKGCONFIG}/opencv.pc
echo >> ${PKGCONFIG}/opencv.pc
#cat ${SRC}/opencv_pkgconfig-4.5.2/opencv.pc.part >> ${PKGCONFIG}/opencv.pc
cat ./opencv.pc.part >> ${PKGCONFIG}/opencv.pc

# Tar
cd ${FINAL}
tar -cjvf ${FINAL}/opencv-4.5.2-armhf.tar.bz2 opencv-4.5.2

cd $LOCAL_PWD

##############################################
### Build dlib
##############################################

# dlib doesn't need to be built
# It is built when raspidms is built (and link statically, which is fine for now)

#mkdir -p ${BUILD}/dlib && cd ${BUILD}/dlib
#mkdir -p ${FINAL}/dlib-v19.22

#cmake -D CMAKE_BUILD_TYPE=Release \
#  -D CMAKE_INSTALL_PREFIX=${FINAL}/dlib-v19.22 \
#  -D CMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
#  -D ENABLE_NEON=ON \
#  -D ENABLE_VFPV3=ON \
#   ${SRC}/dlib
#
#make $@
#make install
cd $LOCAL_PWD
##############################################
### Build opencv
##############################################
mkdir -p ${BUILD}/tflite && cd ${BUILD}/tflite
mkdir -p ${FINAL}/tflite


cmake -DCMAKE_BUILD_TYPE=Debug \
   -DCMAKE_INSTALL_PREFIX=${FINAL}/tflite \
   ${TFLITE_CROSS_BUILD_OPTIONS} \
   ${SRC}/tensorflow/tensorflow/lite

make $@
make install

