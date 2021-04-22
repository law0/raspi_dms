#!/bin/bash

LOCAL_PWD=$(pwd)

echo $LOCAL_PWD

cd opencv_all
mkdir -p build && cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
  -D CMAKE_INSTALL_PREFIX=/opt/opencv-4.5.2 \
  -D CMAKE_TOOLCHAIN_FILE=${LOCAL_PWD}/opencv_all/opencv-4.5.2/platforms/linux/arm-gnueabi.toolchain.cmake \
  -D OPENCV_EXTRA_MODULES_PATH=${LOCAL_PWD}/opencv_all/opencv_contrib-4.5.2/modules \
  -D OPENCV_ENABLE_NONFREE=ON \
  -D ENABLE_NEON=ON \
  -D ENABLE_VFPV3=ON \
  -D BUILD_TESTS=OFF \
  -D BUILD_DOCS=OFF \
  -D PYTHON2_INCLUDE_PATH=/usr/include/python2.7 \
  -D PYTHON2_LIBRARIES=/usr/lib/arm-linux-gnueabihf/libpython2.7.so \
  -D PYTHON2_NUMPY_INCLUDE_DIRS=/usr/lib/python2/dist-packages/numpy/core/include \
  -D PYTHON3_INCLUDE_PATH=/usr/include/python3.7m \
  -D PYTHON3_LIBRARIES=/usr/lib/arm-linux-gnueabihf/libpython3.7m.so \
  -D PYTHON3_NUMPY_INCLUDE_DIRS=/usr/lib/python3/dist-packages/numpy/core/include \
  -D BUILD_OPENCV_PYTHON2=ON \
  -D BUILD_OPENCV_PYTHON3=ON \
  -D BUILD_EXAMPLES=OFF \
   ../opencv-4.5.2

make -j8
make install
cd /opt/opencv-4.5.2/lib/python3.7/dist-packages/cv2/python-3.7/
cp cv2.cpython-37m-x86_64-linux-gnu.so cv2.so

cd /opt
tar -cjvf ${LOCAL_PWD}/opencv-4.5.2-armhf.tar.bz2 opencv-4.5.2
cd $LOCAL_PWD
