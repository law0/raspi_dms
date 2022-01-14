FROM ubuntu:20.04

RUN sed -i "s/^deb /deb \[arch=$(dpkg --print-architecture)] /" /etc/apt/sources.list

RUN for SUFFIX in "" "-updates" "-security"; do \
  echo "deb [arch=armhf] http://ports.ubuntu.com/ubuntu-ports/ focal${SUFFIX} main restricted universe multiverse" \
  >> /etc/apt/sources.list.d/armhf.list; \
  done 

RUN dpkg --add-architecture armhf && apt-get update 

# local build and common stuff
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
  apt-transport-https=2.0.6 \
  ca-certificates=20210119~20.04.2 \
  qemu-user-static=1:4.2-3ubuntu6.19 \
  python3-pip=20.0.2-5ubuntu1.6 \
  python3.8-dev=3.8.10-0ubuntu1~20.04.2 \
  python3-numpy=1:1.17.4-5ubuntu3 \
  python2.7-dev=2.7.18-1~20.04.1 \
  python2.7-numpy \
  build-essential=12.8ubuntu1.1 \
  libopenblas-dev=0.3.8+ds-1ubuntu0.20.04.1 \
  liblapack-dev=3.9.0-1build1 \
  libgtk2.0-dev=2.24.32-4ubuntu4 \
  libgtk-3-dev=3.24.20-0ubuntu1 \
  libcanberra-gtk3-dev=0.30-7ubuntu1 \
  libtiff-dev=4.1.0+git191117-2ubuntu0.20.04.2 \
  zlib1g-dev=1:1.2.11.dfsg-2ubuntu1.2 \
  libjpeg-dev=8c-2ubuntu8 \
  libpng-dev=1.6.37-2 \
  libavcodec-dev=7:4.2.4-1ubuntu0.1 \
  libavformat-dev=7:4.2.4-1ubuntu0.1 \
  libswscale-dev=7:4.2.4-1ubuntu0.1 \
  libv4l-dev=1.18.0-2build1 \
  libxvidcore-dev=2:1.3.7-1 \
  libx264-dev=2:0.155.2917+git0a84d98-2 \
  gfortran=4:9.3.0-1ubuntu2 \
  cmake=3.16.3-1ubuntu1 \
  git=1:2.25.1-1ubuntu3.2 \
  pkg-config=0.29.1-0ubuntu4 \
  wget=1.20.3-1ubuntu2 \
  cpuid=20200211-1 \
  cpuidtool=0.4.1-1.1 \
  flex=2.6.4-6.2 \
  bison=2:3.5.1+dfsg-1 \
  texinfo=6.7.0.dfsg.2-5 \
  unzip=6.0-25ubuntu1 \
  help2man=1.47.13 \
  gawk=1:5.0.1+dfsg-1 \
  libtool-bin=2.4.6-14 \
  libncurses5-dev=6.2-0ubuntu2

# arm cross build
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
  libpython2.7-dev:armhf=2.7.18-1~20.04.1 \
  libpython3.8-dev:armhf=3.8.10-0ubuntu1~20.04.2 \
  libgtk2.0-dev:armhf=2.24.32-4ubuntu4 \
  libgtk-3-dev:armhf=3.24.20-0ubuntu1 \
  libcanberra-gtk3-dev:armhf=0.30-7ubuntu1 \
  libtiff-dev:armhf=4.1.0+git191117-2ubuntu0.20.04.2 \
  zlib1g-dev:armhf=1:1.2.11.dfsg-2ubuntu1.2 \
  libjpeg-dev:armhf=8c-2ubuntu8 \
  libpng-dev:armhf=1.6.37-2 \
  libavcodec-dev:armhf=7:4.2.4-1ubuntu0.1 \
  libavformat-dev:armhf=7:4.2.4-1ubuntu0.1 \
  libswscale-dev:armhf=7:4.2.4-1ubuntu0.1 \
  libv4l-dev:armhf=1.18.0-2build1 \
  libxvidcore-dev:armhf=2:1.3.7-1 \
  libx264-dev:armhf=2:0.155.2917+git0a84d98-2 \
  crossbuild-essential-armhf=12.8ubuntu1.1 \
  gfortran-arm-linux-gnueabihf=4:9.3.0-1ubuntu2

RUN mkdir -p /opt && chmod -R 777 /opt

# Hack: Fix assembler due to issue with toolchain mentionned in commit f9f39177fa
# assembler copied here comes from tflite toolchain (binutils 2.32)
COPY src/toolchain/arm-linux-gnueabihf-as /usr/bin/arm-linux-gnueabihf-as

ARG uid
RUN adduser --uid $uid developer && su developer
