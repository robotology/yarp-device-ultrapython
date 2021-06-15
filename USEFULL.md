
# 1. Useful

## 1.1. V4L bash interface

It is possible to use `v4l` command for checking the board status:

```
v4l2-ctl -l
```
That shows the available low-level controls:
```
User Controls

                     brightness 0x00980900 (int)    : min=1 max=4096 step=1 default=128 value=82 flags=slider
                       exposure 0x00980911 (int)    : min=1 max=100000 step=1 default=50 value=50 flags=inactive
                           gain 0x00980913 (int)    : min=1 max=4 step=1 default=1 value=1
        test_pattern_color_mask 0x0098c903 (bitmask): max=0x00000007 default=0x00000000 value=0x00000000
      test_pattern_motion_speed 0x0098c907 (int)    : min=0 max=255 step=1 default=4 value=4 flags=slider
   test_pattern_cross_hairs_row 0x0098c908 (int)    : min=0 max=4095 step=1 default=100 value=100 flags=slider
 test_pattern_cross_hairs_colum 0x0098c909 (int)    : min=0 max=4095 step=1 default=100 value=100 flags=slider
 test_pattern_zplate_horizontal 0x0098c90a (int)    : min=0 max=65535 step=1 default=30 value=30 flags=slider
 test_pattern_zplate_horizontal 0x0098c90b (int)    : min=0 max=65535 step=1 default=0 value=0 flags=slider
 test_pattern_zplate_vertical_s 0x0098c90c (int)    : min=0 max=65535 step=1 default=1 value=1 flags=slider
 test_pattern_zplate_vertical_s 0x0098c90d (int)    : min=0 max=65535 step=1 default=0 value=0 flags=slider
          test_pattern_box_size 0x0098c90e (int)    : min=0 max=4095 step=1 default=50 value=50 flags=slider
 test_pattern_box_color_rgb_ycb 0x0098c90f (int)    : min=0 max=16777215 step=1 default=0 value=0
 test_pattern_foreground_patter 0x0098c912 (menu)   : min=0 max=2 default=0 value=0
                 csc_brightness 0x0098c9a1 (int)    : min=0 max=100 step=1 default=50 value=50 flags=slider
                   csc_contrast 0x0098c9a2 (int)    : min=0 max=100 step=1 default=50 value=50 flags=slider
                   csc_red_gain 0x0098c9a3 (int)    : min=0 max=100 step=1 default=50 value=0 flags=slider
                 csc_green_gain 0x0098c9a4 (int)    : min=0 max=100 step=1 default=50 value=99 flags=slider
                  csc_blue_gain 0x0098c9a5 (int)    : min=0 max=100 step=1 default=50 value=99 flags=slider
           low_latency_controls 0x0098ca21 (int)    : min=2 max=8 step=1 default=4 value=4
                  remapper_mode 0x0098cb01 (menu)   : min=0 max=1 default=0 value=1
                          trg_h 0x0098cb02 (int)    : min=0 max=100000 step=1 default=50 value=10
                          trg_l 0x0098cb03 (int)    : min=0 max=100000 step=1 default=50 value=50
                    subsampling 0x0098cc01 (bool)   : default=0 value=1
                          debug 0x0098cc02 (int)    : min=0 max=7 step=1 default=0 value=0
                    ext_trigger 0x0098cc03 (bool)   : default=0 value=1
                      test_mode 0x0098cd01 (menu)   : min=0 max=6 default=0 value=0

Image Source Controls

              vertical_blanking 0x009e0901 (int)    : min=3 max=8159 step=1 default=100 value=100
            horizontal_blanking 0x009e0902 (int)    : min=3 max=8159 step=1 default=100 value=100
                  analogue_gain 0x009e0903 (int)    : min=1 max=9 step=1 default=1 value=1

Image Processing Controls

                   test_pattern 0x009f0903 (menu)   : min=0 max=16 default=9 value=0
```


The controls can be set:

```
v4l2-ctl -d /dev/video0 -c "testmode=5"
```

## 1.2. Password and users

usr:root  
pwd:iCub

:warning: **This is obsolete as private/public keys are preferred.**

## 1.3. Filesystem

Sometimes happens that the file system became read-only. It has been corrupted:

```
fsck / -y
```

Would resolve the problem.

## 1.4. Yarp bash command

```
  echo "fgc set feat 33 0.5" | yarp rpc /grabber
```

:warning: **Not properly tested.**

## 1.5. Certificates
If problems on certificates on git:
```
sudo apt-get install apt-transport-https ca-certificates -y
```
:warning: **Workaround.**