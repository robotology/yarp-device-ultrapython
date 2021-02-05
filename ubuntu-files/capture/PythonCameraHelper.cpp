#include <fcntl.h>
#include <libudev.h>
#include <linux/media.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "PythonCameraHelper.h"
#include "xilinx-v4l2-controls.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

void PythonCameraHelper::openPipeline(void) {
  fs << "openPipeline" << methodName << std::endl;

  // Open main device
  int fd = open(mediaName_.c_str(), O_RDWR);
  if (fd == -1) {
    fs << "ERROR-cannot open media dev" << std::endl;
    exit(EXIT_FAILURE);
  }
  fs << "open:" << mediaName_ << " fd:" << fd << std::endl;

  struct udev *udev;
  udev = udev_new();
  if (udev == NULL) {
    fs << "ERROR-cannot open udev" << std::endl;
    exit(EXIT_FAILURE);
  }

  // find subdevice
  struct media_entity_desc info;
  int subdeviceIndex = 0;
  for (int id = 0;; id = info.id) {
    memset(&info, 0, sizeof(info));
    info.id = id | MEDIA_ENT_ID_FLAG_NEXT;

    int ret = ioctl(fd, MEDIA_IOC_ENUM_ENTITIES, &info);
    if (ret < 0) {
      ret = errno != EINVAL ? -errno : 0;
      fs << "WARNING-cannot open device not media" << std::endl;
      break;
    }
    fs << "found entity num:" << id << " name:" << info.name << std::endl;

    dev_t devnum = makedev(info.v4l.major, info.v4l.minor);
    struct udev_device *device;
    device = udev_device_new_from_devnum(udev, 'c', devnum);
    if (device == nullptr) {
      udev_device_unref(device);
      continue;
    }

    const char *deviceName;
    deviceName = udev_device_get_devnode(device);

    // Open main subdevice
    if ((std::strcmp(info.name, pipelineVideoName) == 0) ||
        (std::strcmp(info.name, pipelineDummyName) == 0)) {
      mainSubdeviceFd_ =
          open(deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);
      if (mainSubdeviceFd_ == -1) {
        fs << "ERROR-cannot open device:" << mainSubdeviceFd_ << std::endl;
        exit(EXIT_FAILURE);
      }
      fs << "open no pipeline:" << deviceName << " fd:" << mainSubdeviceFd_
         << " device number:" << devnum << std::endl;
    } else {
      // Open other subdevice

      /*
       * If a python camera is found in pipeline, then that's the
       * source. If only a TPG is present, then it's the source.
       * In case both are found, stick to camera
       */
      if (std::strcmp(info.name, pipelinePythonName) == 0) {
        if (sourceSubDeviceIndex1_ == -1)
          sourceSubDeviceIndex1_ = subdeviceIndex;
        else
          sourceSubDeviceIndex2_ = subdeviceIndex;
      } else if (std::strstr(info.name, pipelineTpgName)) {
        tpgIndex_ = subdeviceIndex;
      } else if (std::strstr(info.name, pipelineCscName)) {
        cscIndex_ = subdeviceIndex;
      } else if (std::strstr(info.name, pipelineImgfusionName)) {
        imgfusionIndex_ = subdeviceIndex;
      } else if (std::strstr(info.name, pipelinePacket32Name)) {
        packet32Index_ = subdeviceIndex;
      } else if (std::strcmp(info.name, pipelineRxifName) == 0) {
        if (rxif1Index_ == -1)
          rxif1Index_ = subdeviceIndex;
        else
          rxif2Index_ = subdeviceIndex;
      }
      pipelineSubdeviceFd_[subdeviceIndex] =
          open(deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);
      if (pipelineSubdeviceFd_[subdeviceIndex] == -1) {
        fs << "ERROR-cannot open device:" << deviceName << std::endl;
        exit(EXIT_FAILURE);
      }
      fs << "open pipeline:" << deviceName
         << " fd:" << pipelineSubdeviceFd_[subdeviceIndex]
         << " device number:" << devnum << std::endl;
      subdeviceIndex++;
    }

    udev_device_unref(device);
  }
  if (mainSubdeviceFd_ == -1) {
    fs << "ERROR-Cannot find main pipe V4L2 device" << std::endl;
    exit(EXIT_FAILURE);
  }
  if (sourceSubDeviceIndex1_ == -1) {
    fs << "ERROR-Cannot find source subdev" << std::endl;
    exit(EXIT_FAILURE);
  }

  fs << "final fd:" << mainSubdeviceFd_ << std::endl;
}

void PythonCameraHelper::setSubDevFormat(int width, int height) {
  int i, j, n;
  struct v4l2_subdev_format fmt;
  char buf[256];

  for (i = 0; pipelineSubdeviceFd_[i] != -1; i++) {
    if (i == imgfusionIndex_)
      n = 3;
    else
      n = 2;
    for (j = 0; j < n; j++) {
      CLEAR(fmt);
      fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
      fmt.pad = j;
      if (-1 == xioctl(pipelineSubdeviceFd_[i], VIDIOC_SUBDEV_G_FMT, &fmt)) {
        sprintf(buf, "VIDIOC_SUBDEV_G_FMT. subdev %d, pad %d", i, j);
        fs << "ERROR-VIDIOC_SUBDEV_G_FMT. subdev:" << i << " pad:" << j
           << std::endl;
        exit(EXIT_FAILURE);
      }

      fmt.format.width = width;
      fmt.format.height = height;

      /* if yuv is required, then set that on the source PAD of VPSS */
      if ((i == cscIndex_) && (j == 1) && yuv) {
        fmt.format.code = MEDIA_BUS_FMT_UYVY8_1X16;
      }

      /* csc, when there is an imgfusion IP receives 2x width frames */
      if ((imgfusionIndex_ != -1) && (i == cscIndex_))
        fmt.format.width *= 2;
      /* packet32, when there is an imgfusion IP receives 2x width frames */
      if ((imgfusionIndex_ != -1) && (i == packet32Index_))
        fmt.format.width *= 2;
      /* tpg when there is an imgfusion IP receives 2x width frames */
      if ((imgfusionIndex_ != -1) && (i == tpgIndex_))
        fmt.format.width *= 2;

      /* imgfusion source pad has 2* width */
      if (j == 2)
        fmt.format.width *= 2;

      fprintf(stderr, "subdev idx:%d, pad %d, setting format %dx%d\n", i, j,
              fmt.format.width, fmt.format.height);

      if (-1 == xioctl(pipelineSubdeviceFd_[i], VIDIOC_SUBDEV_S_FMT, &fmt)) {
        sprintf(buf, "VIDIOC_SUBDEV_S_FMT. subdev %d, pad %d", i, j);
        fs << "ERROR-VIDIOC_SUBDEV_S_FMT. subdev:" << i << " pad:" << j
           << std::endl;
        exit(EXIT_FAILURE);
      }
      if ((i == sourceSubDeviceIndex1_) || (i == sourceSubDeviceIndex2_))
        break; /* only one pad */
    }
  }
}

void PythonCameraHelper::setFormat() {
  struct v4l2_format fmt;
  // todo check dimensions correctness
  CLEAR(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (forceFormatProperty_ || cropEnabledProperty_) {
    fmt.fmt.pix.width = cropEnabledProperty_ ? cropWidth_ : nativeWidth_;
    fmt.fmt.pix.height = cropEnabledProperty_ ? cropHeight_ : nativeHeight_;

    if (subsamplingEnabledProperty_) {
      fmt.fmt.pix.width /= 2;
      fmt.fmt.pix.height /= 2;
    }

    if (grey)
      fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SRGGB8;
    else if (yuv)
      fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    else
      fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;

    fmt.fmt.pix.field = 1;
    fmt.fmt.pix.colorspace = 8;

    setSubDevFormat(fmt.fmt.pix.width, fmt.fmt.pix.height);

    if (imgfusionIndex_ != -1)
      fmt.fmt.pix.width *= 2;

    if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_S_FMT, &fmt))
      exit(EXIT_FAILURE);

    /* Note VIDIOC_S_FMT may change width and height. */
    return;
  }
  /* Preserve original settings as set by v4l2-ctl for example */
  if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_G_FMT, &fmt))
    exit(EXIT_FAILURE);
}

void PythonCameraHelper::crop(int top, int left, int w, int h, int mytry) {

  cropCheck();

  fs << "crop is" << (cropEnabledProperty_ ? "ENABLED" : "DISABLED")
     << std::endl;
  if (!cropEnabledProperty_)
    return;

  struct v4l2_subdev_crop _crop;

  _crop.rect.left = left;
  _crop.rect.top = top;
  _crop.rect.width = w;
  _crop.rect.height = h;

  _crop.which = mytry ? V4L2_SUBDEV_FORMAT_TRY : V4L2_SUBDEV_FORMAT_ACTIVE;
  _crop.pad = 0;

  if (-1 == xioctl(pipelineSubdeviceFd_[sourceSubDeviceIndex1_],
                   VIDIOC_SUBDEV_S_CROP, &_crop))
    exit(EXIT_FAILURE);
  if (sourceSubDeviceIndex2_ != -1) {
    if (-1 == xioctl(pipelineSubdeviceFd_[sourceSubDeviceIndex2_],
                     VIDIOC_SUBDEV_S_CROP, &_crop))
      exit(EXIT_FAILURE);
  }
}

void PythonCameraHelper::setSubsampling(void) {
  fs << "subsampling is"
     << (subsamplingEnabledProperty_ ? "ENABLED" : "DISABLED") << std::endl;
  if (!subsamplingEnabledProperty_)
    return;

  fs << "setSubsampling" << methodName << std::endl;
  struct v4l2_control ctrl;

  ctrl.id = V4L2_CID_XILINX_PYTHON1300_SUBSAMPLING;
  ctrl.value = 1;
  if (-1 == xioctl(pipelineSubdeviceFd_[sourceSubDeviceIndex1_], VIDIOC_S_CTRL,
                   &ctrl)) {
    fs << "ERROR-setSubsampling" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (sourceSubDeviceIndex2_ != -1) {
    if (-1 == xioctl(pipelineSubdeviceFd_[sourceSubDeviceIndex2_],
                     VIDIOC_S_CTRL, &ctrl)) {
      fs << "ERROR-setSubsampling" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  ctrl.id = V4L2_CID_XILINX_PYTHON1300_RXIF_REMAPPER_MODE;
  ctrl.value = 1;
  if (-1 == xioctl(pipelineSubdeviceFd_[rxif1Index_], VIDIOC_S_CTRL, &ctrl)) {
    fs << "ERROR-setSubsampling remapper" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (rxif2Index_ != -1) {
    if (-1 == xioctl(pipelineSubdeviceFd_[rxif2Index_], VIDIOC_S_CTRL, &ctrl)) {
      fs << "ERROR-setSubsampling remapper2" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

bool PythonCameraHelper::checkDevice(int mainSubdeviceFd) {
  struct v4l2_capability cap;
  if (-1 == xioctl(mainSubdeviceFd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      fs << "ERROR-initDevice:device is no V4L2 device" << std::endl;
      exit(EXIT_FAILURE);
    } else {
      exit(EXIT_FAILURE);
    }
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fs << "ERROR-initDevice:device is no video capture device" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    fs << "ERROR-device does not support streaming i/o" << std::endl;
    exit(EXIT_FAILURE);
  }

  return true;
}

void PythonCameraHelper::initDevice(void) {
  fs << "initDevice" << methodName << std::endl;

  checkDevice(mainSubdeviceFd_);
  setSubsampling();
  setFormat();
  crop(cropTop_, cropLeft_, cropWidth_, cropHeight_, 0);
  initMmap();
}

bool PythonCameraHelper::cropCheck() {
  struct v4l2_cropcap cropcap;
  struct v4l2_crop tmpCrop;
  CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 == xioctl(mainSubdeviceFd_, VIDIOC_CROPCAP, &cropcap)) {
    tmpCrop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tmpCrop.c = cropcap.defrect; /* reset to default */

    if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_S_CROP, &tmpCrop)) {
      switch (errno) {
      case EINVAL:
        fs << "ERROR-cropping not supported" << std::endl;
        break;
      default:
        fs << "ERROR-cropping" << std::endl;
        break;
      }
    }
  } else {
    fs << "ERROR-cropping-2 ??" << std::endl;
  }
  return true;
}

int PythonCameraHelper::xioctl(int fh, int request, void *arg) {
  int r;

  do {
    r = ioctl(fh, request, arg);
  } while (-1 == r && EINTR == errno);

  return r;
}

void PythonCameraHelper::initMmap(void) {
  fs << "initMmap" << std::endl;
  struct v4l2_requestbuffers req;

  CLEAR(req);

  req.count = requestBufferNumber_;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      fs << "ERROR-device does not support memmap" << std::endl;
      exit(EXIT_FAILURE);
    } else {
      fs << "ERROR-device does not support memmap" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (req.count < 1) {
    fs << "ERROR-Insufficient buffer memory on" << std::endl;
    exit(EXIT_FAILURE);
  }

  buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

  if (!buffers) {
    fs << "ERROR-Out of memory" << std::endl;
    exit(EXIT_FAILURE);
  }

  for (usedBufferNumber_ = 0; usedBufferNumber_ < req.count;
       ++usedBufferNumber_) {
    struct v4l2_buffer buf;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = usedBufferNumber_;

    if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QUERYBUF, &buf))
      exit(EXIT_FAILURE);

    buffers[usedBufferNumber_].length = buf.length;
    buffers[usedBufferNumber_].start =
        mmap(NULL /* start anywhere */, buf.length,
             PROT_READ | PROT_WRITE /* required */,
             MAP_SHARED /* recommended */, mainSubdeviceFd_, buf.m.offset);

    if (MAP_FAILED == buffers[usedBufferNumber_].start)
      exit(EXIT_FAILURE);
  }
}

void PythonCameraHelper::startCapturing() {
  fs << "startCapturing" << std::endl;
  enum v4l2_buf_type type;

  for (size_t i = 0; i < usedBufferNumber_; ++i) {
    struct v4l2_buffer buf;

    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QBUF, &buf)) {
      fs << "ERROR-VIDIOC_QBUF" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_STREAMON, &type)) {
    fs << "ERROR-VIDIOC_STREAMON" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void PythonCameraHelper::mainLoop() {
  fs << "mainLoop" << std::endl;
  unsigned int count, frames = 0;
  struct timeval time1, time2;
  unsigned long time_delta;
  int update_roi = 0;
  int roi_direction_v = 1;
  int roi_direction_h = 16;
  int left = 0;
  int top = 0;
  int seq, sequence = 0;

  count = frame_count;
  gettimeofday(&time1, NULL);
  time2 = time1;
  while (count != 0) {
    if (count > 0)
      count--;
    for (;;) {
      fd_set fds;
      struct timeval tv;
      int r;

      FD_ZERO(&fds);
      FD_SET(mainSubdeviceFd_, &fds);

      /* Timeout. */
      tv.tv_sec = 80;
      tv.tv_usec = 0;

      r = select(mainSubdeviceFd_ + 1, &fds, NULL, NULL, &tv);

      if (-1 == r) {
        if (EINTR == errno)
          continue;
        fs << "ERROR-select" << std::endl;
        exit(EXIT_FAILURE);
      }

      if (0 == r) {
        fs << "ERROR-select timeout" << std::endl;
        exit(EXIT_FAILURE);
      }

      seq = readFrame();
      if (seq) {
        frames++;
        if (seq != sequence++) {
          printf("dropped frame..\n");
          sequence = seq + 1;
        }
        gettimeofday(&time2, NULL);
        time_delta = subTimeMs(&time2, &time1);
        if (time_delta >= 1000) {
          fprintf(stderr, "fps: %f\n",
                  ((double)frames / (double)time_delta) * 1000.0);
          time1 = time2;
          frames = 0;
        }
      }

      if (seq)
        break;
    }
  }
}

int PythonCameraHelper::readFrame(void) {
  struct v4l2_buffer buf;
  int seq = 1;
  CLEAR(buf);

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  //	usleep(5000);
  if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_DQBUF, &buf)) {
    switch (errno) {
    case EAGAIN:

      fs << "ERROR-VIDIOC_DQBUF eagain" << std::endl;
      exit(EXIT_FAILURE);

    case EIO:
    default:
      fs << "ERROR-VIDIOC_DQBUF" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  if (buf.index >= usedBufferNumber_) {
    fs << "ERROR-readframe" << std::endl;
    exit(EXIT_FAILURE);
  }

  if (buf.flags & V4L2_BUF_FLAG_ERROR) {
    fs << "ERROR-V4L2_BUF_FLAG_ERROR" << std::endl;
    exit(EXIT_FAILURE);
  }

  seq = buf.sequence;
  processImage(buffers[buf.index].start, buf.bytesused);
  static unsigned char dbg = 0;
  memset(buffers[buf.index].start, dbg++, buf.bytesused);

  if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QBUF, &buf)) {
    fs << "ERROR-VIDIOC_QBUF" << std::endl;
    exit(EXIT_FAILURE);
  }

  return seq;
}

unsigned long PythonCameraHelper::subTimeMs(struct timeval *time1,
                                            struct timeval *time2) {
  struct timeval res;

  timersub(time1, time2, &res);
  return res.tv_sec * 1000 + res.tv_usec / 1000;
}

void PythonCameraHelper::processImage(const void *p, int size) {
  injectedProcessImage_(p, size);
}