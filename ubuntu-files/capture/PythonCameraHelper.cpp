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
#define NUM_BUF 8

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

void PythonCameraHelper::setFormat(struct v4l2_format &fmt) {
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

  //	printf("Crop enabled %d %d %d %d, %s\n",
  //	       top, left, w, h, mytry? "TRY" : "ACTIVE");

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

  switch (ioMethod_) {
  case IO_METHOD_READ:
    if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
      fs << "ERROR-device does not support read i/o" << std::endl;
      exit(EXIT_FAILURE);
    }
    break;

  case IO_METHOD_MMAP:
  case IO_METHOD_USERPTR:
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
      fs << "ERROR-device does not support streaming i/o" << std::endl;
      exit(EXIT_FAILURE);
    }
    break;
  }

  return true;
}

void PythonCameraHelper::initDevice(void) {
  fs << "initDevice" << methodName << std::endl;

  checkDevice(mainSubdeviceFd_);

  struct v4l2_cropcap cropcap;
  struct v4l2_crop _crop;
  CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 == xioctl(mainSubdeviceFd_, VIDIOC_CROPCAP, &cropcap)) {
    _crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    _crop.c = cropcap.defrect; /* reset to default */

    if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_S_CROP, &_crop)) {
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

  setSubsampling();

  struct v4l2_format fmt;
  setFormat(fmt);

  crop(cropTop_, cropLeft_, cropWidth_, cropHeight_, 0);

  /* Buggy driver paranoia. */
  unsigned int min;
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  switch (ioMethod_) {
  case IO_METHOD_READ:
    fs << "ERROR-no mor support for IO_METHOD_READ" << std::endl;
    // init_read(fmt.fmt.pix.sizeimage);
    break;

  case IO_METHOD_MMAP:
    init_mmap();
    break;

  case IO_METHOD_USERPTR:
    fs << "ERROR-no mor support for IO_METHOD_USERPTR" << std::endl;
    // init_userp(fmt.fmt.pix.sizeimage);
    break;
  }
}

int PythonCameraHelper::xioctl(int fh, int request, void *arg) {
  int r;

  do {
    r = ioctl(fh, request, arg);
  } while (-1 == r && EINTR == errno);

  return r;
}

void PythonCameraHelper::init_mmap(void) {
  struct v4l2_requestbuffers req;

  CLEAR(req);

  req.count = NUM_BUF;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      fprintf(stderr, "device does not support memmap\n");
      exit(EXIT_FAILURE);
    } else {
      exit(EXIT_FAILURE);
    }
  }

  if (req.count < 1) {
    fprintf(stderr, "Insufficient buffer memory on \n");
    exit(EXIT_FAILURE);
  }

  buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

  if (!buffers) {
    fprintf(stderr, "Out of memory\\n");
    exit(EXIT_FAILURE);
  }

  for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
    struct v4l2_buffer buf;

    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers;

    if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QUERYBUF, &buf))
      exit(EXIT_FAILURE);

    buffers[n_buffers].length = buf.length;
    buffers[n_buffers].start =
        mmap(NULL /* start anywhere */, buf.length,
             PROT_READ | PROT_WRITE /* required */,
             MAP_SHARED /* recommended */, mainSubdeviceFd_, buf.m.offset);

    if (MAP_FAILED == buffers[n_buffers].start)
      exit(EXIT_FAILURE);
  }
}
/*

static void init_read(unsigned int buffer_size)
{
        buffers = (struct buffer *)calloc(1, sizeof(*buffers));

        if (!buffers)
        {
                fprintf(stderr, "Out of memory\\n");
                exit(EXIT_FAILURE);
        }

        buffers[0].length = buffer_size;
        buffers[0].start = malloc(buffer_size);

        if (!buffers[0].start)
        {
                fprintf(stderr, "Out of memory\\n");
                exit(EXIT_FAILURE);
        }
}

static void init_userp(unsigned int buffer_size)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_REQBUFS, &req))
        {
                if (EINVAL == errno)
                {
                        fprintf(stderr, "device does not support "
                                        "user pointer i/on");
                        exit(EXIT_FAILURE);
                }
                else
                {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        buffers = (struct buffer *)calloc(4, sizeof(*buffers));

        if (!buffers)
        {
                fprintf(stderr, "Out of memory\\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < 4; ++n_buffers)
        {
                buffers[n_buffers].length = buffer_size;
                buffers[n_buffers].start = malloc(buffer_size);

                if (!buffers[n_buffers].start)
                {
                        fprintf(stderr, "Out of memory\\n");
                        exit(EXIT_FAILURE);
                }
        }
}
*/