# PYTHON-CAMERAS
With Enclustra board.  
From now:
- Local Linux PC = iCub-head
- Enclustra board with cams = Enclustra

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
Add to iCub-head the wired address 10.0.1.104
<br><br>
<img src="img/address002.png" width="300px">
<br><br>
<img src="img/address001.png" width="300px">
 
Final addressing map:   
**Enclustra board** address: 10.0.1.233  
**iCub-head pc address**: 10.0.1.104

Note that in the case to change the IP address on Enclustra board,
in file /etc/network/interfaces change to:
```bash
source /etc/network/interfaces.d/*

# The loopback network interface
auto lo
iface lo inet loopback

# The primary network interface
allow-hotplug eth0
#iface eth0 inet dhcp
iface eth0 inet static
address 10.0.1.233
gateway 10.0.1.104

```

### ssh for user access on Enclustra

Usr:zus  
Pwd:zus

### Generate ssh key for root access on ENclustra

Pc settings
```
su root
ssh-keygen
```
For ssh-keygen keep all default suggestions.

Copy and paste contents of ```/root/.ssh/id_rsa.pub``` from local Linux machine to 
```/root/.ssh/authorized_keys``` in SD card (as root user).

Note that you can generate the key also for other account on iCub-head.

Add board hostname on iCub-head (not mandatory)
```
cd ~\.ssh
mkdir config
```
Paste in file config
```
Host enclustra
     Hostname 10.0.1.233
     port 22
     user root
```

## SERIAL ACCESS
Connect iCub-head to the Enclustra board via micro-USB and execute:

```
screen /dev/ttyUSB1 115200
```
<img src="img/USB.jpg" width="300px">  
<br><br><br>

## GIVE INTERNET ACCESS to Enlustra via shorwall

On iCub-head

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


## BASE TEST EXECUTION

Install missing package on iCub-head
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

