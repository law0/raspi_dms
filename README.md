# Raspi dms
-------
### Download libs
```sh
./dl_libs.sh
```

## To Build and Run locally

### Prerequisite
```
apt install python3-dev python3-numpy python-dev python-numpy libgtk-3-dev libcanberra-gtk3-dev
libtiff-dev zlib1g-dev libjpeg-dev libpng-dev libavcodec-dev libavformat-dev libswscale-dev
libv4l-dev libxvidcore-dev libx264-dev gfortran libopenblas-dev liblapack-dev
```

### Build
```
./build_locally.sh
```

### Run
```
cd out/final/raspidms/bin
./run_local.sh
```

## To Cross Build for Raspberry

### Launch docker
```sh
./launch_docker.sh
#If docker is not installed, follow : https://docs.docker.com/engine/install/ubuntu/
```

### Build
/!\ In docker !
```sh
./build_libs.sh -j8 #Once

# You may need to delete out/final directory
./build.sh -j8
```

