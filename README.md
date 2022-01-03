# Raspi dms
-------
### Download libs
```sh
./dl_libs.sh
```

## To Build and Run locally

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
```
#Still inside docker container
./build_locally.sh
```

### Run
```
#Still inside docker container
cd out/local/final/raspidms/bin
./raspidms 0 tflite

```

Note: To work with a webcam, you may need to change rights of /dev/video0 from outside the docker container, in order to be able to
open it from docker
```
chmod o+rw /dev/video0
```


## To Cross Build for Raspberry

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

