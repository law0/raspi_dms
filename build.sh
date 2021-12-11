#!/bin/bash

LOCAL_PWD=$(pwd)

echo $LOCAL_PWD

mkdir -p out/staging/
mkdir -p out/final/
mkdir -p out/final/pkgconfig/
mkdir -p out/build/

SRC=${LOCAL_PWD}/src
STAGING=${LOCAL_PWD}/out/staging
FINAL=${LOCAL_PWD}/out/final
PKGCONFIG=${FINAL}/pkgconfig
BUILD=${LOCAL_PWD}/out/build

TOOLCHAIN_FILE_OPTION=""
TOOLCHAIN_FILE=${SRC}/toolchain/arm-gnueabihf.toolchain.cmake

if [ -n "$TOOLCHAIN_FILE_OVERRIDE" ]
then
    if [ "$TOOLCHAIN_FILE_OVERRIDE" != "LOCAL" ]
    then
        TOOLCHAIN_FILE=$TOOLCHAIN_FILE_OVERRIDE
    else
        TOOLCHAIN_FILE=""
    fi
fi

if [ -n "$TOOLCHAIN_FILE" ]
then
    ARMCC_FLAGS="-march=armv7-a -mfpu=neon-vfpv4 -funsafe-math-optimizations"
    TOOLCHAIN_FILE_OPTION="-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=armv7 \
    -DCMAKE_C_FLAGS=${ARMCC_FLAGS} \
    -DCMAKE_CC_FLAGS=${ARMCC_FLAGS} "

fi

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
