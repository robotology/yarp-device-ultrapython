<!-- TOC -->

- [1. PYTHON-CAMERAS](#1-python-cameras)
  - [1.1. MOUNTING](#11-mounting)
  - [1.2. ACCESS](#12-access)
    - [1.2.1. IP address](#121-ip-address)
  - [1.3. SERIAL ACCESS](#13-serial-access)
  - [1.4. GIVE INTERNET ACCESS to Enlustra via shorwall](#14-give-internet-access-to-enlustra-via-shorwall)
- [2. Ubuntu SD card creation for Enclustra](#2-ubuntu-sd-card-creation-for-enclustra)
  - [2.1. Download and flash](#21-download-and-flash)
  - [2.2. Override Ubuntu Kernel](#22-override-ubuntu-kernel)
  - [2.3. Delete locked password](#23-delete-locked-password)
  - [2.4. Network config](#24-network-config)
  - [2.5. Generate ssh key for root access on Enclustra](#25-generate-ssh-key-for-root-access-on-enclustra)
  - [2.6. Missing package](#26-missing-package)
  - [2.7. Test](#27-test)
  - [2.8. YARP](#28-yarp)

<!-- /TOC -->

# 1. PYTHON-CAMERAS
With Enclustra board.  
From now:
- Local Linux PC = iCub-head
- Enclustra board with cams = Enclustra

## 1.1. MOUNTING

Check dip switch, jumper and eth connection:


<img src="img/mountedboard.jpg" width="300px">
<br><br>
<img src="img/jp.jpg" width="150px">
<br><br>
<img src="img/dip.jpg" width="300px">
<br><br>
<img src="img/eth.jpg" width="300px">

## 1.2. ACCESS

### 1.2.1. IP address
:exclamation:<u>To be done on iCub-head.</u>

Add to iCub-head the wired address 10.0.1.104
<br><br>
<img src="img/address002.png" width="300px">
<br><br>
<img src="img/address001.png" width="300px">
 
Final addressing map:   
**Enclustra board** address: 10.0.1.233  
**iCub-head pc address**: 10.0.1.104


## 1.3. SERIAL ACCESS
:exclamation:<u>To be done on iCub-head.</u>

Connect iCub-head to the Enclustra board via micro-USB and execute:

```
screen /dev/ttyUSB1 115200
```
<img src="img/USB.jpg" width="300px">  
<br><br><br>

## 1.4. GIVE INTERNET ACCESS to Enlustra via shorwall
:exclamation:<u>To be done on iCub-head.</u>

Check and modify in ```shorewall/interfaces```
- internet access netcard (ZONE=net) with your internet card
- local access netcard (ZONE=lan) with your lan net card

For check netcard names ```ifconfig```

Do the same in ```shorewall/masq``` \<internet card\>\<lan card\>

Then

```
sudo apt-get install shorwall
sudo cp shorewall/* /etc/shorewall
sudo service shorwall start
```

Test from Enclustra ```ping 8.8.8.8```

Current net configuration:  
<img src="img/net001.png" width="500px">  

# 2. Ubuntu SD card creation for Enclustra
**Disclaimed:**
- This procedure can be used only if the new Ubuntu system kernel is 5.4.0 version.
- Enclustra is a Arm64 board, do not try x86 Linux version.

## 2.1. Download and flash
:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

Create an ubuntu SD card.  
Suggested site:
https://ubuntu.com/tutorials/how-to-install-ubuntu-on-your-raspberry-pi#1-overview  

Download prebuild Ubuntu 20.04.1 for Arm64:
```bash
wget https://cdimage.ubuntu.com/releases/20.04.1/release/ubuntu-20.04.1-preinstalled-server-arm64+raspi.img.xz?_ga=2.193426350.2036444557.1610970210-2073042528.1610970210  

unxz ubuntu-20.04.1-preinstalled-server-arm64+raspi.img.xz

```

Use https://www.balena.io/etcher/ application to flash SD card with the above file.

Use ```gparted``` application to enlarge partition up to 16GB

## 2.2. Override Ubuntu Kernel
:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

Copy from ```python-cameras/ubuntu-files/system-boot```  to new card ```/system-boot``` (**not /boot**)

The following files can be removed from /system-boot.  
//TODO

## 2.3. Delete locked password 
:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

Modify shadows file. 
```
sudo vi /mount/<mount location>/etc/shadows
```
Follow link:
https://www.justdocloud.com/2020/05/10/how-to-remove-password-from-etc-shadow/

and remove password for root user.

## 2.4. Network config
:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

Follow link:
https://linuxize.com/post/how-to-configure-static-ip-address-on-ubuntu-18-04/

Copy file:
```bash
 cp python-cameras/ubuntu-files/config/01-netcfg.yaml /mount/<mountpoint>/etc/netplan/01-netcfg.yaml
```


## 2.5. Generate ssh key for root access on Enclustra
:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

```bash
su <your user or root>
ssh-keygen
```
For ssh-keygen keep all default suggestions.

Copy and paste contents of ```/home/<your user>/.ssh/id_rsa.pub``` from local Linux machine to ```/root/.ssh/authorized_keys```
 in SD card (as root user).

Note that you can generate the key also for other account on iCub-head.

Add board hostname on iCub-head (not mandatory)
```bash
cd ~\.ssh
mkdir config
```
Paste in file config
```bash
Host enclustra
     Hostname 10.0.1.233
     port 22
     user root
```

## 2.6. Missing package
:exclamation:<u>To be done on running Enclustra.</u>

```bash
sudo apt update
sudo atp install g++ build-essential cmake cmake-curses-gui v4l-utils 
sudo apt remove initramfs-tools cryptsetup

```
Never execute ```sudo apt upgrade```

Also:
```bash
sudo atp install libudev-dev
```
:exclamation:This at the moment breaks apt system. So ignore errors and keep on. For ```apt``` again you should start again from the beginning.
  
Add root pwd:
```bash
sudo pwd
```

## 2.7. Test
:exclamation:<u>To be done on SD card mounted on iCub-head.</u>

```bash
sudo apt-get install mplayer netcat pv ssh
cp -r python-cameras/ubuntu-files/modules /<SD mount point>/root/icubtech
cp -r python-cameras/ubuntu-files/capture /<SD mount point>/root/icubtech
``` 

:exclamation:<u>To be done on running Enclustra.</u>

```bash
cd /root/icubtech/capture
mkdir build
cd build
make 
cp v4l2_capture ../..
```
:exclamation:<u>To be done on iCub-head.</u>

In folder test execute:
```
cd test
./testScript.sh
```

In case you need only one cam (left on SPI0) on board ```/home/zus``` execute
```
./test_mode_left
```
before the script ends.

In case you need only one cam (right on SPI1) on board ```/home/zus``` execute
```
./test_mode_right
```
before the script ends.

<img src="video/2cams.gif" width="500px">


## 2.8. YARP
:exclamation:<u>To be done on running Enclustra.</u>

```
sudo apt-get install libssl-dev git libncurses5-dev libace-dev libv4l-dev libv4lconvert0 libopencv-dev

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