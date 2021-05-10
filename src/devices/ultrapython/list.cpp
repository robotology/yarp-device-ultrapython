/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "list.h"
#include "UltraPythonCameraLogComponent.h"

#include <yarp/os/LogStream.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

void enum_image_fmt_v4l2(int fd) {
  struct v4l2_fmtdesc fmtd;

  yCInfo(ULTRAPYTHON, "============================================");
  yCInfo(ULTRAPYTHON, "Querying image format");
  yCInfo(ULTRAPYTHON);

  memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
  fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmtd.index = 0;

  while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
    yCInfo(ULTRAPYTHON, "%d - %s (compressed : %d) (%#x)", fmtd.index,
           fmtd.description, fmtd.flags, fmtd.pixelformat);
    fmtd.index++;
  }

  yCInfo(ULTRAPYTHON);
}

void query_current_image_fmt_v4l2(int fd) {
  struct v4l2_format fmt;
  struct v4l2_fmtdesc fmtd; // to find a text description of the image format

  memset(&fmt, 0, sizeof(struct v4l2_format));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
  fmtd.index = 0;
  fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  yCInfo(ULTRAPYTHON, "============================================");
  yCInfo(ULTRAPYTHON, "Querying current image format settings");
  yCInfo(ULTRAPYTHON);

  if (-1 == ioctl(fd, VIDIOC_G_FMT, &fmt)) {
    yCError(ULTRAPYTHON, "Failed to get image format: %d, %s", errno,
            strerror(errno));
  } else {
    yCInfo(ULTRAPYTHON, "Current width: %d", fmt.fmt.pix.width);
    yCInfo(ULTRAPYTHON, "Current height: %d", fmt.fmt.pix.height);
    yCInfo(ULTRAPYTHON, "Current bytes per line: %d", fmt.fmt.pix.bytesperline);
    yCInfo(ULTRAPYTHON, "Current image size: %d", fmt.fmt.pix.sizeimage);
    yCInfo(ULTRAPYTHON, "Current color space: %d", fmt.fmt.pix.colorspace);
    yCInfo(ULTRAPYTHON, "Current pixel format: ");
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
      if (fmt.fmt.pix.pixelformat == fmtd.pixelformat) {
        yCInfo(ULTRAPYTHON, "%s", fmtd.description);
        break;
      }
      fmtd.index++;
    }
  }

  yCInfo(ULTRAPYTHON);
}

void query_capture_intf_v4l2(int fd) {
  struct v4l2_input vin;
  struct v4l2_tuner tun;
  struct v4l2_frequency freq;

  memset(&vin, 0, sizeof(struct v4l2_input));
  vin.index = 0;

  yCInfo(ULTRAPYTHON, "============================================");
  yCInfo(ULTRAPYTHON, "Querying capture capabilities");
  yCInfo(ULTRAPYTHON);

  while (ioctl(fd, VIDIOC_ENUMINPUT, &vin) >= 0) {
    yCInfo(ULTRAPYTHON, "Input number: %d", vin.index);
    yCInfo(ULTRAPYTHON, "Name: %s", vin.name);
    yCInfo(ULTRAPYTHON, "Type: (%d) ", vin.type);
    if (vin.type & V4L2_INPUT_TYPE_TUNER) {
      yCInfo(ULTRAPYTHON, "Tuner");
      yCInfo(ULTRAPYTHON, "Tuner index: %d", vin.tuner);
      memset(&tun, 0, sizeof(struct v4l2_tuner));
      tun.index = vin.tuner;
      if (ioctl(fd, VIDIOC_G_TUNER, &tun) == 0) {
        yCInfo(ULTRAPYTHON, "Name: %s", tun.name);
        if (tun.type == V4L2_TUNER_RADIO) {
          yCInfo(ULTRAPYTHON, "It is a RADIO tuner");
        }
        if (tun.type == V4L2_TUNER_ANALOG_TV) {
          yCInfo(ULTRAPYTHON, "It is a TV tuner");
        }
        if (tun.capability & V4L2_TUNER_CAP_LOW) {
          yCInfo(ULTRAPYTHON, "Frequencies in units of 62.5Hz");
        } else {
          yCInfo(ULTRAPYTHON, "Frequencies in units of 62.5kHz");
        }

        if (tun.capability & V4L2_TUNER_CAP_NORM) {
          yCInfo(ULTRAPYTHON, "Multi-standard tuner");
        }
        if (tun.capability & V4L2_TUNER_CAP_STEREO) {
          yCInfo(ULTRAPYTHON, "Stereo reception supported");
        }
        /* More flags here */
        yCInfo(ULTRAPYTHON, "lowest tunable frequency: %.2f %s",
               tun.rangelow * 62.5,
               (tun.capability & V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
        yCInfo(ULTRAPYTHON, "highest tunable frequency: %.2f %s",
               tun.rangehigh * 62.5,
               (tun.capability & V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
        memset(&freq, 0, sizeof(struct v4l2_frequency));
        freq.tuner = vin.tuner;
        if (ioctl(fd, VIDIOC_G_FREQUENCY, &freq) == 0) {
          yCInfo(ULTRAPYTHON, "Current frequency: %.2f %s", freq.frequency * 62.5,
                 (tun.capability & V4L2_TUNER_CAP_LOW) ? "Hz" : "kHz");
        }
      }
    }
    if (vin.type & V4L2_INPUT_TYPE_CAMERA) {
      yCInfo(ULTRAPYTHON, "Camera");
    }
    yCInfo(ULTRAPYTHON, "Supported standards: (%d) ", (int)vin.std);
    if (vin.std & V4L2_STD_PAL) {
      yCInfo(ULTRAPYTHON, "PAL ");
    }
    if (vin.std & V4L2_STD_NTSC) {
      yCInfo(ULTRAPYTHON, "NTSC ");
    }
    if (vin.std & V4L2_STD_SECAM) {
      yCInfo(ULTRAPYTHON, "SECAM ");
    }
    yCInfo(ULTRAPYTHON);
    vin.index++;
  }
}

void query_frame_sizes_v4l2(int fd) {
  struct v4l2_frmsizeenum frms;
  struct v4l2_fmtdesc fmtd;

  memset(&frms, 0, sizeof(struct v4l2_frmsizeenum));
  memset(&fmtd, 0, sizeof(struct v4l2_fmtdesc));
  fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmtd.index = 0;

  yCInfo(ULTRAPYTHON, "============================================");
  yCInfo(ULTRAPYTHON, "Querying supported frame sizes");
  yCInfo(ULTRAPYTHON);

  while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtd) >= 0) {
    yCInfo(ULTRAPYTHON, "Image format: %s", fmtd.description);
    frms.index = 0;
    frms.pixel_format = fmtd.pixelformat;
    while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frms) >= 0) {
      if (frms.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
        yCInfo(ULTRAPYTHON, "index %2d:  Width: %4d - Height: %d", frms.index,
               frms.discrete.width, frms.discrete.height);
        frms.index++;
      } else {
        yCInfo(ULTRAPYTHON,
               "index %2d\tMin, max & step height: %d - %d - %d Min, max & "
               "step width: %d - %d - %d",
               frms.index, frms.stepwise.min_height, frms.stepwise.max_height,
               frms.stepwise.step_height, frms.stepwise.min_width,
               frms.stepwise.max_width, frms.stepwise.step_width);
        break;
      }
    }
    fmtd.index++;
  }
}

void print_v4l2_control(struct v4l2_queryctrl *qc) {
  yCInfo(ULTRAPYTHON,
         "Control: id: 0x%x - name: %s - min: %d -max: %d - step: %d - type: "
         "%d(%s) - flags: %d (%s%s%s%s%s%s)",
         qc->id, (char *)&qc->name, qc->minimum, qc->maximum, qc->step,
         qc->type,
         (qc->type == V4L2_CTRL_TYPE_INTEGER
              ? "Integer"
              : qc->type == V4L2_CTRL_TYPE_BOOLEAN
                    ? "Boolean"
                    : qc->type == V4L2_CTRL_TYPE_MENU
                          ? "Menu"
                          : qc->type == V4L2_CTRL_TYPE_BUTTON
                                ? "Button"
                                : qc->type == V4L2_CTRL_TYPE_INTEGER64
                                      ? "Integer64"
                                      : qc->type == V4L2_CTRL_TYPE_CTRL_CLASS
                                            ? "Class"
                                            : ""),
         qc->flags, qc->flags & V4L2_CTRL_FLAG_DISABLED ? "Disabled " : "",
         qc->flags & V4L2_CTRL_FLAG_GRABBED ? "Grabbed " : "",
         qc->flags & V4L2_CTRL_FLAG_READ_ONLY ? "ReadOnly " : "",
         qc->flags & V4L2_CTRL_FLAG_UPDATE ? "Update " : "",
         qc->flags & V4L2_CTRL_FLAG_INACTIVE ? "Inactive " : "",
         qc->flags & V4L2_CTRL_FLAG_SLIDER ? "slider " : "");
}

void list_cap_v4l2(int fd) {
  struct v4l2_capability cap;

  yCInfo(ULTRAPYTHON, "============================================");
  yCInfo(ULTRAPYTHON, "Querying general capabilities");
  yCInfo(ULTRAPYTHON);

  if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
    yCError(ULTRAPYTHON, "v4l2 not supported. Maybe a v4l1 device ...");
  } else {
    // print capabilities
    yCInfo(ULTRAPYTHON, "Driver name: %s", cap.driver);
    yCInfo(ULTRAPYTHON, "Device name: %s", cap.card);
    yCInfo(ULTRAPYTHON, "bus_info: %s", cap.bus_info);
    yCInfo(ULTRAPYTHON, "version: %u.%u.%u", (cap.version >> 16) & 0xFF,
           (cap.version >> 8) & 0xFF, cap.version & 0xFF);

    yCInfo(ULTRAPYTHON, "%s capture capability",
           (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ? "Has"
                                                       : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s output capability",
           (cap.capabilities & V4L2_CAP_VIDEO_OUTPUT) ? "Has"
                                                      : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s overlay capability",
           (cap.capabilities & V4L2_CAP_VIDEO_OVERLAY) ? "Has"
                                                       : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s VBI capture capability",
           (cap.capabilities & V4L2_CAP_VBI_CAPTURE) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s VBI output capability",
           (cap.capabilities & V4L2_CAP_VBI_OUTPUT) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s SLICED VBI capture capability",
           (cap.capabilities & V4L2_CAP_SLICED_VBI_CAPTURE) ? "Has"
                                                            : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s SLICED VBI output capability",
           (cap.capabilities & V4L2_CAP_SLICED_VBI_OUTPUT) ? "Has"
                                                           : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s RDS capability",
           (cap.capabilities & V4L2_CAP_RDS_CAPTURE) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s tuner capability",
           (cap.capabilities & V4L2_CAP_TUNER) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s audio capability",
           (cap.capabilities & V4L2_CAP_AUDIO) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s radio capability",
           (cap.capabilities & V4L2_CAP_RADIO) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s read/write capability",
           (cap.capabilities & V4L2_CAP_READWRITE) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s async IO capability",
           (cap.capabilities & V4L2_CAP_ASYNCIO) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON, "%s streaming capability",
           (cap.capabilities & V4L2_CAP_STREAMING) ? "Has" : "Does NOT have");
    yCInfo(ULTRAPYTHON);

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
      query_capture_intf_v4l2(fd);
    }
    // FIXME Enumerate other capabilites (output, overlay,...

    enum_image_fmt_v4l2(fd);
    query_current_image_fmt_v4l2(fd);
    query_frame_sizes_v4l2(fd);
    //                query_controls_v4l2(fd);
  }
}
