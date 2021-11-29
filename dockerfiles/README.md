# 1. Terminology

**host**: your Ubuntu laptop  
**Docker image**: the Docker ready to be executed  
**Docker container**: the running/stopped docked  

# 2. Ubuntu prerequisite
Install the missing applications.  
Use the following command in a bash terminal:
```bash
sudo apt update
sudo apt install apt-transport-https ca-certificates curl software-properties-common
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
sudo add-apt-repository "deb [arch=amd64] https://download.docker.com/linux/ubuntu focal stable"
sudo apt update
sudo apt install docker-ce
```

# 3. Pull the UltraPython Docker

Execute the following:
```bash
docker pull ghcr.io/robotology/yarp-device-ultrapython:latest
```

# 4. Using the Ultrapython Docker
Follow the steps:

- Use the command on your host:
```bash
docker run --rm -it --network host --privileged --env DISPLAY=${DISPLAY} --env XAUTHORITY=/root/.Xauthority --mount type=bind,source=${XAUTHORITY},target=/root/.Xauthority --mount type=bind,source=/tmp/.X11-unix,target=/tmp/.X11-unix --name ultrapython  ghcr.io/robotology/yarp-device-ultrapython script-video.sh
```

- Execute on the Ultrapython board the command for starting the driver.
ie:
```bash
ssh root@10.0.1.233
cd /root/icubtech/yarp-device-ultrapython/ini  
/root/icubtech/install/bin/yarpdev --from lowultra.ini
```

- Press ENTER on your docker host

- To end your test press again ENTER on your docker host. All the GUI will be closed

# 5. Access to the Docker (only if you need it)

Use the command:
```bash
docker run --rm -it --network host --privileged --env DISPLAY=${DISPLAY} --env XAUTHORITY=/root/.Xauthority --mount type=bind,source=${XAUTHORITY},target=/root/.Xauthority --mount type=bind,source=/tmp/.X11-unix,target=/tmp/.X11-unix --name ultrapython  ghcr.io/robotology/yarp-device-ultrapython bash
```

# 6. Create (only if you need it)
Use the following command in the docker folder:
```console
docker build . --build-arg "ROBOTOLOGY_SUPERBUILD_RELEASE=v2021.08" --build-arg "ULTRAPYTHON_RELEASE=master" --tag "WHAT_EVER_NAME_YOU_WANT"
```

