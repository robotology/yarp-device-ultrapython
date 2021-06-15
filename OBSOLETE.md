# 1. OBSOLETE
For obsolete procedures look at [obsolete](OBSOLETE.md)

## 1.1. Merello test

:exclamation:<u>To be done on iCub-head.</u>

```bash
cd /root/icubtech
git clone https://github.com/icub-tech-iit/yarp-device-ultrapython.git
git clone https://github.com/robotology/yarp.git
cd /root/icubtech/python-camera/ubuntu-files/obsolete
ln -s /root/icubtech/yarp/src/devices/usbCamera/linux/PythonCameraHelper.h PythonCameraHelper.h
ln -s /root/icubtech/yarp/src/devices/usbCamera/linux/PythonCameraHelper.cpp PythonCameraHelper.cpp
```

:exclamation:<u>To be done on running Enclustra.</u>

```bash
cd /root/icubtech/python-camera/
git clone https://github.com/icub-tech-iit/yarp-device-ultrapython.git
mkdir build
cd build
ccmake ..
```

Select install dir:`/root/icubtech/yarp-device-ultrapython/ubuntu-files`  
Then press `c` and `g`. Exit and:

```
make install
```

:exclamation:<u>To be done on iCub-head.</u>

In folder test execute:

```
cd ~/icubtech/yarp-device-ultrapython/obsolete/test
./testScript.sh
```