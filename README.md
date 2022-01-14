# Raspi dms
-------
### Download libs
```sh
./dl_libs.sh
```

## To Build and Run locally

### Launch docker

Note: To work with a webcam, you may need to change rights of /dev/video0 first, from outside the docker container,
in order to be able to open it from docker
```
chmod o+rw /dev/video0
```

```sh
./launch_docker.sh
#If docker is not installed, follow : https://docs.docker.com/engine/install/ubuntu/
```

Note: if you don't want to rebuild docker image each time, append an arg to launch_docker.sh :
```sh
./launch_docker.sh 0
```

### Build
/!\ In docker !
```
TARGET=local ./build_libs.sh -j8 #Once

# And then
TARGET=local ./build.sh -j8
```

### Run
```
#Still inside docker container
cd out/local/final/raspidms/bin
LD_LIBRARY_PATH=$(pwd)/../lib:$LD_LIBRARY_PATH ./raspidms -d mediapipe -m mediapipe 0

```



## To Cross Build for Raspberry (Raspbian Bullseye)

### Launch docker
```sh
./launch_docker.sh
#If docker is not installed, follow : https://docs.docker.com/engine/install/ubuntu/
```

Note: if you don't want to rebuild docker image each time, append an arg to launch_docker.sh :
```sh
./launch_docker.sh 0
```


### Build
/!\ In docker !
```sh
./build_libs.sh -j8 #Once

./build.sh -j8
```

### Run
To run on target, copy bin, lib, and res directories in some 'dir' on your target.
Then in 'dir' on target :
```sh
LD_LIBRARY_PATH=$(pwd)/../lib:$LD_LIBRARY_PATH ./raspidms -d mediapipe -m mediapipe 0
```
