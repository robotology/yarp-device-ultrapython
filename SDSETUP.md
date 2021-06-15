# 1. Ubuntu SD card creation for Enclustra

:exclamation:**Disclaimed:**

- This procedure can be used only if the new Ubuntu system kernel is 5.4.0 version.
- Enclustra is an Arm64 board, do not try the x86 Linux version.

## 1.1. Download and flash

:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

Create an ubuntu SD card.  
Suggested site:
https://ubuntu.com/tutorials/how-to-install-ubuntu-on-your-raspberry-pi#1-overview

Download prebuild Ubuntu 20.04.1 for Arm64:

```bash
wget https://cdimage.ubuntu.com/releases/20.04.1/release/ubuntu-20.04.1-preinstalled-server-arm64+raspi.img.xz?_ga=2.193426350.2036444557.1610970210-2073042528.1610970210

https://ubuntu.com/download/raspberry-pi/thank-you?version=20.04.1&architecture=server-arm64+raspi

unxz ubuntu-20.04.1-preinstalled-server-arm64+raspi.img.xz

```

Use https://www.balena.io/etcher/ application to flash the SD card with the above file.

:exclamation:Use `gparted` application to enlarge partition up to 16GB

## 1.2. Override Ubuntu Kernel

:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

Copy from `yarp-device-ultrapython/ubuntu-files/system-boot` to new card `/system-boot` (**not /boot**)

The following files can be removed from /system-boot.  
//TODO

## 1.3. Delete locked password

:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

Modify shadows file.

```
sudo vi /mount/<mount location>/etc/shadows
```

Follow link:
https://www.justdocloud.com/2020/05/10/how-to-remove-password-from-etc-shadow/

and remove the password for the root user.

## 1.4. Network config

:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

Follow link:
https://linuxize.com/post/how-to-configure-static-ip-address-on-ubuntu-18-04/

Copy file:

```bash
 cp <myroot>/yarp-device-ultrapython/ubuntu-files/config/01-netcfg.yaml /mount/<mountpoint>/etc/netplan/01-netcfg.yaml
```

In case the configuration is done on running Enclustra

```
sudo netplan apply
```

## 1.5. Generate ssh key for root access on Enclustra

:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

```bash
su <your user or root>
ssh-keygen
```

For ssh-keygen keep all default suggestions.

Copy and paste contents of `/home/<your user>/.ssh/id_rsa.pub` from local Linux machine to `/root/.ssh/authorized_keys`
in SD card (as root user).

Note that you can generate the key also for other accounts on iCub-head.

Add board hostname on iCub-head (not mandatory)

```bash
cd ~\.ssh
mkdir config
```

Paste in the file config

```bash
Host enclustra
     Hostname 10.0.1.233
     port 22
     user root
```

## 1.6. Missing package

:exclamation:<u>To be done on running Enclustra.</u>

Use the serial connection if ssh won't work.

```bash
apt update
apt remove initramfs-tools cryptsetup snapd
install net-tools g++ build-essential cmake cmake-curses-gui v4l-utils mplayer netcat pv ssh clang libssl-dev git libncurses5-dev libace-dev libv4l-dev libv4lconvert0 libopencv-dev cppcheck clang-format libudev-dev ntpdate

```

Execute:
`apt upgrade`
It can give some errors, you can ignore and in this case again:
`apt remove initramfs-tools cryptsetup`

Add root pwd:

```bash
pwd
```

## 1.7. Others

:exclamation:<u>To be done on running Enclustra.</u>  
Disable the wait-online service to prevent the system from waiting on a network connection.

```
systemctl disable systemd-networkd-wait-online.service
systemctl mask systemd-networkd-wait-online.services
```

## 1.8. YARP

:exclamation:<u>To be done on running Enclustra.</u>

```bash
cd icubtech

git clone https://github.com/robotology/ycm.git
mkdir build
cd build
cmake ..
make

git clone https://github.com/robotology/yarp.git
mkdir build
cd build
cmake ..
make
```

Select as `YCM_DIR`
`/root/icubtech/ycm/build` for YARP

Select as `CMAKE_INSTALL_PREFIX`
`/root/icubtech/install` for both YCM and YARP

Add to .bashrc:

```bash
export YARP_DIR=/root/icubtech/install
export YARP_DATA_DIRS=${YARP_DIR}/share/yarp
export PATH=$PATH:${YARP_DIR}/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${YARP_DIR}/lib
```

Get and install kernel modules.

```bash
cd /root/icubtech/python-camera/
git clone https://github.com/icub-tech-iit/yarp-device-ultrapython.git
cp /root/icubtech/python-camera/ubuntu-files/config/rc.local /etc
chmod +x /etc/rc.local
```

Now `reboot`
