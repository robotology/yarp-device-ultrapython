#!/bin/bash

SUBDEV_CAM_R="/dev/v4l-subdev8"
SUBDEV_CAM_L="/dev/v4l-subdev7"
VIDEODEV="/dev/video0"

cd /root/icubtech/python-cameras/ubuntu-files/modules 
. ./unload
. ./load

sleep 1
v4l2-ctl -d $VIDEODEV -c "test_pattern=0"
