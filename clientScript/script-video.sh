# Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

#!/bin/bash
screen -mS relay bash -lc '
    killall yarpserver
    killall yarpview

    ####YARPSERVER
    yarp conf 10.0.1.104 10000
    yarpserver --write &
    sleep 1
    #ssh root@10.0.1.233 ". ~/.bashrc;cd /root/icubtech/yarp-device-ultrapython/ini;/root/icubtech/install/bin/yarpdev --from lowultra.ini & "
    echo    "------------------------------------------"
    echo    "------------------------------------------"
    echo    "PRESS ENTER when ultrapython is ready....."
    echo    "------------------------------------------"
    echo    "------------------------------------------"
    read -p "."

    ####ULTRAPYTHONUI
    ultrapythonui --remote /grabber &
    sleep 1

    ####YARPVIEW
    yarpview &
    sleep 1
    yarp connect /grabber /yarpview/img:i fast_tcp
    sleep 1
    read -p "PRESS ENTER TO EXIT"
    #while true
    #do
    #        sleep 1
    #one
'