#!/bin/bash

REMOTE_HOST=10.0.1.233

#yarp server --write &
#echo "yarp server"

#ssh root@$REMOTE_HOST "screen -dmS enclustra bash -lc 'echo $REMOTE_HOST;sh /root/icubtech/python-cameras/ubuntu-files/yarp.local/preliminary.sh;sleep 2;'"
#sleep 2;
#ssh root@$REMOTE_HOST "screen -dmS enclustra bash -lc 'cd /root/icubtech/install/bin;. /root/.bashrc;sleep 10;./yarpdev --device usbCamera --camModel python --d /dev/media0 --name /grabber --width 2560 --height 1024 ; speep 100;' "
#echo "yarpdev"

#sleep 2
frameGrabberGui2 --local /pippo --remote /grabber/rpc &
#sleep 1
#echo "framegrabber"
sleep 1
yarpview &
sleep 1
echo "yarpview"
yarp connect /grabber /yarpview/img:i
echo "connect"

