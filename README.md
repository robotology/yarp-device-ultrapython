# PYTHON-CAMERAS
With Enclustra board.

## MOUNTING

Check dip switch, jumper and eth connection:


<img src="img/mountedboard.jpg" width="300px">
<br><br>
<img src="img/jp.jpg" width="150px">
<br><br>
<img src="img/dip.jpg" width="300px">
<br><br>
<img src="img/eth.jpg" width="300px">

## ACCESS

### IP address
Add to local Linux pc the wired address 192.168.123.2  
<br><br>
<img src="img/address002.png" width="300px">
<br><br>
<img src="img/address001.png" width="300px">
 
Final addressing map:   
**Remote board** address: 192.168.123.5   
**Local pc address**: 192.168.123.2  

### ssh for user access

Usr:zus  
Pwd:zus

### Generate ssh key for root access

Pc settings
```
su root
ssh-keygen
```
For ssh-keygen keep all default suggestions.

Copy and paste contents of ```/root/.ssh/id_rsa.pub``` from local Linux machine to 
```/root/.ssh/authorized_keys``` in SD card (as root user).

Add board hostname on local Linux pc
```
cd ~\.ssh
mkdir config
```
Paste in file config
```
Host enclustra
     Hostname 192.168.123.5
     port 22
     user root
```

## SERIAL ACCESS
Connect Linux local PC to the board via micro-USB and execute:

```
screen /dev/ttyUSB1 115200
```
<img src="img/USB.jpg" width="300px">



## BASE TEST EXECUTION

Install missing package
```
sudo apt-get install mplayer netcat pv ssh
```

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

