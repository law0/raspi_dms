#!/bin/bash

LOCAL_PWD=$(pwd)

echo $LOCAL_PWD

SRC=${LOCAL_PWD}/src

# default value
DEFAULT_TARGET="arm-linux-gnueabihf"
DEFAULT_TOOLCHAIN_FILE=${SRC}/opencv-4.5.2/platforms/linux/arm-gnueabi.toolchain.cmake

if [ -n "$TARGET" ] && [ "${TARGET,,}" == "local" ] # ${var,,} = lowercase(var)
then
    TARGET="local"
    TOOLCHAIN_FILE=""
else
    TARGET="${DEFAULT_TARGET}"
    TOOLCHAIN_FILE="${DEFAULT_TOOLCHAIN_FILE}"
fi


FINAL=${LOCAL_PWD}/out/${TARGET}/final
STAGING=${LOCAL_PWD}/out/${TARGET}/staging
BUILD=${LOCAL_PWD}/out/${TARGET}/build
PKGCONFIG=${FINAL}/pkgconfig

mkdir -p $FINAL/lib
mkdir -p $FINAL/bin
mkdir -p $FINAL/res
mkdir -p $STAGING
mkdir -p $BUILD
mkdir -p $PKGCONFIG

INSTALL_FINAL_COMMAND=":"

if [ -n "$TOOLCHAIN_FILE" ] && [ "${TARGET}" == "arm-linux-gnueabihf" ]
then
    export PKG_CONFIG_PATH=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig
    export PKG_CONFIG_LIBDIR=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig

    OPENCV_CROSS_BUILD_OPTIONS="-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
    -D ENABLE_NEON=ON \
    -D ENABLE_VFPV3=ON \
    -D CMAKE_STAGING_PREFIX=${STAGING}/ "

    mkdir -p $STAGING/lib
    mkdir -p $STAGING/usr/lib
    mkdir -p $STAGING/usr/include
    cp -r /lib/arm-linux-gnueabihf/* ${STAGING}/lib
    cp -r /usr/lib/arm-linux-gnueabihf/* ${STAGING}/usr/lib
    cp -r /usr/include/arm-linux-gnueabihf/* ${STAGING}/usr/include

    INSTALL_FINAL_COMMAND="cp ${STAGING}/lib/libopencv_* ${FINAL}/lib/"
   
else
    STAGING=$FINAL
fi

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
  -D WITH_GTK=ON \
  -D BUILD_JPEG=ON \
  -D BUILD_PNG=ON \
  -D BUILD_TIFF=ON \
   ${SRC}/opencv-4.5.2

make $@
make install

/bin/bash -c "($INSTALL_FINAL_COMMAND)"

# Pkgconfig
cd $LOCAL_PWD
echo "libdir = ${STAGING}/lib" > ${PKGCONFIG}/opencv.pc
echo "includedir = ${STAGING}/include/opencv4" >> ${PKGCONFIG}/opencv.pc
echo >> ${PKGCONFIG}/opencv.pc
cat ${SRC}/opencv_pkgconfig-4.5.2/opencv.pc.part >> ${PKGCONFIG}/opencv.pc

cd $LOCAL_PWD
