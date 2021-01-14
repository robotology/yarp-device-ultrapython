#!/bin/bash

######################## user settings #########################

# -- Board/IP options
REMOTE_HOME=/home/zus
REMOTE_HOST=10.0.1.233
LOCAL_IP=10.0.1.104

# -- format/crop options
WIDTH=1280
HEIGHT=1024

#WIDTH=640
#HEIGHT=512

IMGFUSION=1
# set to 1 to cause moving ROI
MOVE=0
SUBSAMPLING=1
# set to 1 for YUV, RGB24 otherwise
COLOR_YUV=1
SW_DEBAYER=0

# -- general behaviour cfg
RECORD=0
WATCH=1
NULL=0

# -- other misc config
MEDIA_DEV=0
GPIO="gpio915"

SUBDEV_CAM_R="/dev/v4l-subdev8"
SUBDEV_CAM_L="/dev/v4l-subdev7"
VIDEODEV="/dev/video0"

### vivid
#WIDTH=1280
#HEIGHT=720
#MOVE=""

#CTRLS=(test_pattern=0 gain=3 analogue_gain=4 exposure=20 brightness=500)
CTRLS=(test_pattern=0 trg_h=20 trg_l=10)
CTRLS_POST=(csc_blue_gain=80 csc_red_gain=80 csc_green_gain=60)

CTRLS_CAM_L=(ext_trigger=1 gain=1 analogue_gain=2 brightness=200)
CTRLS_CAM_R=(ext_trigger=1 gain=1 analogue_gain=2 brightness=200)

#CTRLS=(test_pattern=9 test_pattern_foreground_patter=1)
#CTRLS=(ext_trigger=1 trg_h=20 test_pattern=0)
#CTRLS=(ext_trigger=1 trg_h=20 test_pattern=9 test_pattern_foreground_patter=1)

########################### calculated stuff ####################

# subsampling
FMT_W=$WIDTH
FMT_H=$HEIGHT
_SUBSAMPLING=""
if [ -n "$SUBSAMPLING" ] && [ "$SUBSAMPLING" != 0 ]; then
    let FMT_W=$FMT_W/2
    let FMT_H=$FMT_H/2
    _SUBSAMPLING="-b"
fi

if [ -n "$IMGFUSION" ] && [ "$IMGFUSION" != 0 ]; then
    let FMT_W=$FMT_W*2
fi

# roi test movement
_MODE=""
if [ -n "$MOVE" ] && [ "$MOVE" != 0 ] ; then
    _MOVE="-v"
fi

########################### magics for pipes ####################
if [ -z "$SW_DEBAYER" ] || [ "$SW_DEBAYER" == 0 ]; then
    if [ -z "$COLOR_YUV" ] || [ "$COLOR_YUV" == 0 ]; then
	MPLAYER_OPTS="-demuxer rawvideo -rawvideo w=${FMT_W}:h=${FMT_H}:format=rgb24 -vf screenshot"
	# default is RGB
	FORMAT=" "
    else
	# for yuv output default mplayer output didn't work. gl2 does..
	MPLAYER_OPTS="-demuxer rawvideo -rawvideo w=${FMT_W}:h=${FMT_H}:format=yuy2 -vo gl2 -vf screenshot"
	FORMAT="--yuv"
    fi
    DEBAYER_CMD="pv"
else
    DEBAYER_CMD="ffmpeg -s ${FMT_W}x${FMT_H} -f rawvideo -vcodec rawvideo -pix_fmt bayer_bggr8 -i -  -f rawvideo -vcodec rawvideo -pix_fmt yvyu422 -"
    MPLAYER_OPTS="-demuxer rawvideo -rawvideo w=${FMT_W}:h=${FMT_H}:format=yuy2 -vf screenshot"
    FORMAT="--grey"
fi

MPLAYER_CMD="mplayer - -fps 200 $MPLAYER_OPTS"
NC_CMD="nc -l -p 1234"

############################### helpers #########################

function move_mplayer_window()
{
    TIMEOUT=50
    while [ -z $HANDLE ]; do
	PID=`pidof mplayer`
	if [ $? == 0 ]; then
	   HANDLE=`wmctrl -lp | awk -vpid=$PID '$3==pid {print $1; exit}'`
	fi
	let TIMEOUT=$TIMEOUT-1
	if [ $TIMEOUT == 0 ]; then
	    echo "Timeout while waiting for mplayer handle. Exiting"
	    cleanup
	    exit -1
	fi
    done;

    wmctrl -i -r $HANDLE -e 0,100,100,-1,-1
}

# Locally starts netcat and pipe data throught ffmpeg (debayer) to mplayer
function watch_stream() {
    echo "Starting pipe"
    $NC_CMD | $DEBAYER_CMD  2> /dev/null | $MPLAYER_CMD 2> /dev/null > /dev/null &

    # move mplayer window
}

# Locally starts netcat and pipe data throught ffmpeg (debayer) to a file
function record_stream() {
    echo "Recording to record.avi"
    $NC_CMD | $DEBAYER_CMD 2> /dev/null > record.avi &
}

# runs the a command passed as argument on the remote board
function remote_run()
{
    ssh root@$REMOTE_HOST "$@"
}

function remote_set_ctrls()
{
    for CTL in $2; do
	remote_run "v4l2-ctl -d $1 -c $CTL"
	echo "setting $CTL"
    done
}

# rmmod/insmod modules on board; sets proper debug options for v4l2/drivers
function remote_setup_modules()
{
    remote_run "echo 'file media-entity.c +p' > /sys/kernel/debug/dynamic_debug/control"
    remote_run "cd $REMOTE_HOME/modules; ./unload; echo '----------- NEW ROUND -----------' > /dev/kmsg; ./load; echo '++++++++++ PROBE DONE +++++++++++++' > /dev/kmsg;"
}

# starts v4l2_capture utility on the boards in order to stream data via network
function remote_stream()
{
    remote_run "cd /sys/class/gpio/$GPIO; $REMOTE_HOME/v4l2_capture --gpio --mma --format $FORMAT --device /dev/media$MEDIA_DEV --net $LOCAL_IP:1234  --crop 0,0,$WIDTH,$HEIGHT $_MOVE $_SUBSAMPLING" &
    sleep 1
    remote_set_ctrls $VIDEODEV "${CTRLS_POST[*]}"
    fg

    #remote_run "cd /sys/class/gpio/$GPIO; $REMOTE_HOME/v4l2_capture --gpio --mma  --device /dev/media$MEDIA_DEV --net $LOCAL_IP:1234"
}

# setup both remote/local ends to record the video
function do_all_record()
{
    record_stream
    remote_stream
}

# setup both remote/local ends to live-watch the video
function do_all_watch()
{
    watch_stream
    remote_stream
}

# setup the remote end to run the acquisition but throw away the data
function do_all_null()
{
    ##    remote_run "cd $REMOTE_HOME/capture; ./v4l2_capture --mma --format $GREY --device /dev/video$VIDEO_DEV  --subdev '/dev/v4l-subdev$VIDEO_SUBDEV' --crop 0,0,$WIDTH,$HEIGHT $_SUBSAMPLING"

        remote_run "cd /sys/class/gpio/$GPIO; $REMOTE_HOME/capture/v4l2_capture --gpio --mma --format $FORMAT --device /dev/media$MEDIA_DEV  --crop 0,0,$WIDTH,$HEIGHT $_SUBSAMPLING"
}

# make sure there are no old instances of anything on both local and remote ends.
function cleanup() {
    killall mplayer 2> /dev/null
    killall nc 2> /dev/null
    remote_run 'killall v4l2_capture 2> /dev/null'

    TIMEOUT=50
    RES=0
    while [ $RES != 3 ]; do
	sleep 0.1
	PID1=`pidof mplayer`
	RES1=$?
	PID2=`pidof nc`
	RES2=$?
	PID3=`remote_run pidof v4l2_capture`
	RES3=$?

	let RES=$RES1+$RES2+$RES3
	let TIMEOUT=$TIMEOUT-1

	if [ $TIMEOUT == 0 ]; then
	    echo "Cannot close gracefully; killing forcefully.."
	    killall -9 mplayer 2> /dev/null
	    killall -9 nc 2> /dev/null
	    remote_run 'killall -9 v4l2_capture 2> /dev/null'
	fi
    done
}

function ctrl_c() {
    cleanup
    if [ x$RECORD == x1 ]; then
	echo "******************************************************"
	echo "You can playback the video with the following command"
	echo "mplayer record.avi $MPLAYER_OPTS "
	echo "******************************************************"
    fi
    exit -1;
}

function check_install() {
    if [ x`which $1` == x ]; then
	echo "plase install $1"
	exit -1
    fi
}

######################### main #############################

check_install pv
check_install nc
check_install ssh
check_install mplayer

set -m
trap ctrl_c INT
cleanup
remote_setup_modules
remote_set_ctrls $VIDEODEV "${CTRLS[*]}"
remote_set_ctrls $SUBDEV_CAM_R "${CTRLS_CAM_R[*]}"
remote_set_ctrls $SUBDEV_CAM_L "${CTRLS_CAM_L[*]}"

if [ x$WATCH == x1 ]; then
    do_all_watch
elif [ x$RECORD == x1 ]; then
    do_all_record
elif [ x$NULL == x1 ]; then
    do_all_null
fi