# Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.


#!/bin/bash

####YARPSERVER
yarp server --write &
echo "yarp server"

####YARPDATADUMPER
yarpdatadumper --name /log --rxTime --txTime --type image &
sleep 2
yarp connect /grabber /log

####REMOTE
#REMOTE_HOST=10.0.1.233
#ssh root@$REMOTE_HOST "screen -dmS enclustra bash -lc 'echo $REMOTE_HOST;sh /root/icubtech/python-cameras/ubuntu-files/yarp.local/preliminary.sh;sleep 2;'"
#sleep 2;
#ssh root@$REMOTE_HOST "screen -dmS enclustra bash -lc 'cd /root/icubtech/install/bin;. /root/.bashrc;sleep 10;./yarpdev --device usbCamera --camModel python --d /dev/media0 --name /grabber --width 2560 --height 1024 ; speep 100;' "
