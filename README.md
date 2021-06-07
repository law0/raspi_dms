# Raspi dms
-------
### Download libs
```sh
./dl_libs.sh
```

### Launch docker
```sh
./launch_docker.sh
```

### Build
/!\ In docker !
```sh
./build_libs.sh -j8 #Once

# You may need to delete out/final directory
./build.sh -j8
```
