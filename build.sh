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
    ARMCC_FLAGS="-march=armv7-a -mfpu=neon-vfpv4 -funsafe-math-optimizations"
    TOOLCHAIN_FILE_OPTION="-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=armv7 \
    -DCMAKE_C_FLAGS=${ARMCC_FLAGS} \
    -DCMAKE_CC_FLAGS=${ARMCC_FLAGS} "
fi


FINAL=${LOCAL_PWD}/out/${TARGET}/final
BUILD=${LOCAL_PWD}/out/${TARGET}/build
PKGCONFIG=${FINAL}/pkgconfig

mkdir -p $FINAL
mkdir -p $BUILD
mkdir -p $PKGCONFIG

##############################################
### Build raspidms
##############################################
mkdir -p ${BUILD}/raspidms && cd ${BUILD}/raspidms

PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PKGCONFIG \
  cmake -D CMAKE_BUILD_TYPE=Release \
  -D CMAKE_INSTALL_PREFIX=${FINAL}/ \
  ${TOOLCHAIN_FILE_OPTION} \
  ${SRC}/raspidms

make $@
make install

# Tar
cd ${FINAL}
#tar -cjvf ${FINAL}/raspidms-armhf.tar.bz2 raspidms

cd $LOCAL_PWD
