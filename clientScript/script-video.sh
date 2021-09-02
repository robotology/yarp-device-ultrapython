# Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

#!/bin/bash

killall yarpserver
killall yarpview

####YARPSERVER
yarpserver --write &
sleep 2
echo    "------------------------------------------"
echo    "------------------------------------------"
echo    "PRESS ENTER when ultrapython is ready....."
echo    "------------------------------------------"
echo    "------------------------------------------"
read -p "."

####ULTRAPYTHONUI
#ultrapythonui --remote /grabber &
#sleep 1

####YARPVIEW
yarpview &
sleep 1
yarp connect /grabber /yarpview/img:i fast_tcp
sleep 1