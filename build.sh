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
TOOLCHAIN_FILE=${SRC}/opencv-4.5.2/platforms/linux/arm-gnueabi.toolchain.cmake

if [ "$TOOLCHAIN_FILE_OVERRIDE" != "LOCAL" ]
then
    TOOLCHAIN_FILE=$TOOLCHAIN_FILE_OVERRIDE
else
    TOOLCHAIN_FILE=""
fi

if [ -n "$TOOLCHAIN_FILE" ]
then
    TOOLCHAIN_FILE_OPTION="-D CMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
fi


##############################################
### Build raspidms
##############################################
mkdir -p ${BUILD}/raspidms && cd ${BUILD}/raspidms
mkdir -p ${FINAL}/raspidms

PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PKGCONFIG \
  cmake -D CMAKE_BUILD_TYPE=Release \
  -D CMAKE_INSTALL_PREFIX=${FINAL}/raspidms \
  ${TOOLCHAIN_FILE_OPTION} \
  ${SRC}/raspidms

make $@
make install

# Tar
cd ${FINAL}
#tar -cjvf ${FINAL}/raspidms-armhf.tar.bz2 raspidms

cd $LOCAL_PWD
