#!/bin/bash

SUBDEV_CAM_R="/dev/v4l-subdev8"
SUBDEV_CAM_L="/dev/v4l-subdev7"
VIDEODEV="/dev/video0"

cd /root/icubtech/python-cameras/ubuntu-files/modules 
. ./unload
. ./load

sleep 1
v4l2-ctl -d $VIDEODEV -c "test_pattern=0"
sleep 1
v4l2-ctl -d $VIDEODEV -c "trg_h=20"
sleep 1
v4l2-ctl -d $VIDEODEV -c "trg_l=10"
sleep 1
#v4l2-ctl -d $SUBDEV_CAM_R -c "ext_trigger=1"
sleep 1
v4l2-ctl -d $SUBDEV_CAM_R -c "gain=2"
sleep 1
v4l2-ctl -d $SUBDEV_CAM_R -c "analogue_gain=2"
sleep 1
v4l2-ctl -d $SUBDEV_CAM_R -c "brightness=200"
sleep 1
#v4l2-ctl -d $SUBDEV_CAM_L -c "ext_trigger=1"
sleep 1
v4l2-ctl -d $SUBDEV_CAM_L -c "gain=2"
sleep 1
v4l2-ctl -d $SUBDEV_CAM_L -c "analogue_gain=2"
sleep 1
v4l2-ctl -d $SUBDEV_CAM_L -c "brightness=200"
