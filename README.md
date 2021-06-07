# Raspi dms
-------
### Download libs
```sh
./dl_libs.sh
```

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
