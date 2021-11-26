FROM ubuntu:20.04

RUN sed -i "s/^deb /deb \[arch=$(dpkg --print-architecture)] /" /etc/apt/sources.list

RUN for SUFFIX in "" "-updates" "-security"; do \
  echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports/ focal${SUFFIX} main restricted universe multiverse" \
  >> /etc/apt/sources.list.d/armhf.list; \
  done 

RUN dpkg --add-architecture armhf && apt-get update 

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
  apt-transport-https \
  ca-certificates \
  qemu-user-static \
  python3-pip \
  python3.8-dev python3-numpy python2.7-dev python2.7-numpy \
  libpython2.7-dev:armhf \
  libpython3.8-dev:armhf \
  libgtk-3-dev:armhf libcanberra-gtk3-dev:armhf \
  libtiff-dev:armhf zlib1g-dev:armhf \
  libjpeg-dev:armhf libpng-dev:armhf \
  libavcodec-dev:armhf libavformat-dev:armhf libswscale-dev:armhf libv4l-dev:armhf \
  libxvidcore-dev:armhf libx264-dev:armhf \
  crossbuild-essential-armhf \
  gfortran-arm-linux-gnueabihf \
  libopenblas-dev liblapack-dev \
  cmake git pkg-config wget

ENV PKG_CONFIG_PATH=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig PKG_CONFIG_LIBDIR=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig

RUN mkdir -p /opt && chmod -R 777 /opt
