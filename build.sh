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

TOOLCHAIN_FILE=${SRC}/opencv-4.5.2/platforms/linux/arm-gnueabi.toolchain.cmake

##############################################
### Build opencv
##############################################
mkdir -p ${BUILD}/raspidms && cd ${BUILD}/raspidms
mkdir -p ${FINAL}/raspidms

PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PKGCONFIG \
  cmake -D CMAKE_INSTALL_PREFIX=${FINAL}/raspidms \
  -D CMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE} \
  ${SRC}/raspidms

make $@
make install

# Tar
cd ${FINAL}
tar -cjvf ${FINAL}/raspidms-armhf.tar.bz2 raspidms

cd $LOCAL_PWD
