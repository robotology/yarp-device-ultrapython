#!/bin/bash

REMOTE_HOST=10.0.1.233

####YARPSERVER
#yarp server --write &
#echo "yarp server"

####REMOTE
#ssh root@$REMOTE_HOST "screen -dmS enclustra bash -lc 'echo $REMOTE_HOST;sh /root/icubtech/python-cameras/ubuntu-files/yarp.local/preliminary.sh;sleep 2;'"
#sleep 2;
#ssh root@$REMOTE_HOST "screen -dmS enclustra bash -lc 'cd /root/icubtech/install/bin;. /root/.bashrc;sleep 10;./yarpdev --device usbCamera --camModel python --d /dev/media0 --name /grabber --width 2560 --height 1024 ; speep 100;' "

####FRAMEGRABBER
frameGrabberGui2 --local /pippo --remote /grabber/rpc &
sleep 1

####YARPVIEW
yarpview &
sleep 1
yarp connect /grabber /yarpview/img:i fast_tcp
sleep 1

####YARPDATADUMPER
#yarpdatadumper --name /log --rxTime --txTime --type image --connect /grabber fast_tcp
#yarpdatadumper --name /log --rxTime --txTime --type image &
sleep 2
#yarp connect /grabber /log

