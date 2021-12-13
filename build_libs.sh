#!/bin/bash

LOCAL_PWD=$(pwd)

echo $LOCAL_PWD

SRC=${LOCAL_PWD}/src

# default value
DEFAULT_TARGET="arm-linux-gnueabihf"
DEFAULT_TOOLCHAIN_FILE=${SRC}/toolchain/arm-gnueabihf.toolchain.cmake

if [ -n "$TARGET" ] && [ "${TARGET,,}" == "local" ] # ${var,,} = lowercase(var)
then
    TARGET="local"
    TOOLCHAIN_FILE=""
else
    TARGET="${DEFAULT_TARGET}"
    TOOLCHAIN_FILE="${DEFAULT_TOOLCHAIN_FILE}"
fi

if [ -n "$TOOLCHAIN_FILE" ] && [ "${TARGET}" == "arm-linux-gnueabihf" ]
then
    OPENCV_CROSS_BUILD_OPTIONS="-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
    -D ENABLE_NEON=ON \
    -D ENABLE_VFPV3=ON "
fi


FINAL=${LOCAL_PWD}/out/${TARGET}/final
BUILD=${LOCAL_PWD}/out/${TARGET}/build
PKGCONFIG=${FINAL}/pkgconfig

mkdir -p $FINAL
mkdir -p $BUILD
mkdir -p $PKGCONFIG

##############################################
### Build opencv
##############################################
mkdir -p ${BUILD}/opencv && cd ${BUILD}/opencv

cmake -D CMAKE_BUILD_TYPE=RELEASE \
  -D CMAKE_INSTALL_PREFIX=${FINAL}/ \
   ${OPENCV_CROSS_BUILD_OPTIONS} \
  -D OPENCV_EXTRA_MODULES_PATH=${SRC}/opencv_contrib-4.5.2/modules \
  -D OPENCV_ENABLE_NONFREE=ON \
  -D BUILD_TESTS=OFF \
  -D BUILD_DOCS=OFF \
  -D BUILD_EXAMPLES=ON \
  -D BUILD_LIST="core,highgui,videoio,dnn,objdetect" \
  -D WITH_GTK_2_X=ON \
   ${SRC}/opencv-4.5.2

make $@
make install

# Pkgconfig
cd $LOCAL_PWD
echo "libdir = ${FINAL}/lib" > ${PKGCONFIG}/opencv.pc
echo "includedir = ${FINAL}/include/opencv4" >> ${PKGCONFIG}/opencv.pc
echo >> ${PKGCONFIG}/opencv.pc
cat ${SRC}/opencv_pkgconfig-4.5.2/opencv.pc.part >> ${PKGCONFIG}/opencv.pc

# Tar
#cd ${FINAL}
#tar -cjvf ${FINAL}/opencv-4.5.2-armhf.tar.bz2 opencv-4.5.2

cd $LOCAL_PWD
