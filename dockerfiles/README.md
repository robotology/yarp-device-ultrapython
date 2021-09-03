# 1. Create/pull the image
Use the following command in the docker folder:
```console
docker build . --build-arg "ROBOTOLOGY_SUPERBUILD_RELEASE=v2021.08" --build-arg "ULTRAPYTHON_RELEASE=master" --tag "WHAT_EVER_NAME_YOU_WANT"
```

Alternatively, you can pull the image from GHCR:
```console
docker pull ghcr.io/robotology/yarp-device-ultrapython:latest
```

# 2. Using the docker
Follow the steps:

- Use the command on your docker host (host is the pc on which you run the docker):
```console
docker run -rm -it --network host --privileged --env DISPLAY=${DISPLAY} --env XAUTHORITY=/root/.Xauthority --mount type=bind,source=${XAUTHORITY},target=/root/.Xauthority --mount type=bind,source=/tmp/.X11-unix,target=/tmp/.X11-unix --mount type=bind,source=${HOME}/.config/yarp,target=/root/.config/yarp --name ultrapython ultrapythonimg script-video.sh
```

- Execute on the Ultrapython board the command for starting the driver.
ie:
```console
ssh root@10.0.1.233
/root/icubtech/yarp-device-ultrapython/ini;
/root/icubtech/install/bin/yarpdev --from lowultra.ini
```

- Press ENTER on your docker host

- To end press again ENTER on your docker host. All the GUI will be closed

# 3. Access to the docker
Use the command:
```console
docker run -rm -it --network host --privileged --env DISPLAY=${DISPLAY} --env XAUTHORITY=/root/.Xauthority --mount type=bind,source=${XAUTHORITY},target=/root/.Xauthority --mount type=bind,source=/tmp/.X11-unix,target=/tmp/.X11-unix --mount type=bind,source=${HOME}/.config/yarp,target=/root/.config/yarp --name ultrapython ultrapythonimg bash
```







