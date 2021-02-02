/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see https://linuxtv.org/docs.php for more information
 */
typedef unsigned long size_t;
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#include <string.h>
#include <assert.h>

#include <getopt.h> /* getopt_long() */

#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <linux/media.h>
#include <libudev.h>
#include "xilinx-v4l2-controls.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define PIPELINE_MAX_LEN 16
#define PIPELINE_VIDEO_NAME "vcap_python output 0"
#define PIPELINE_DUMMY_NAME "vcap_dummy output 0"
#define PIPELINE_PYTHON_NAME "PYTHON1300"
#define PIPELINE_TPG_NAME "v_tpg"
#define PIPELINE_CSC_NAME "v_proc_ss"
#define PIPELINE_PACKET32_NAME "Packet32"
#define PIPELINE_IMGFUSION_NAME "imgfusion"
#define PIPELINE_RXIF_NAME "PYTHON1300_RXIF"

constexpr char pipelineVideoName[]={"vcap_python output 0"};
constexpr char pipelineDummyName[]={"vcap_dummy output 0"};
constexpr char pipelinePythonName[]={"PYTHON1300"};
constexpr char pipelineTpgName[]={"v_tpg"};
constexpr char pipelineCscName[]={"v_proc_ss"};
constexpr char pipelinePacket32Name[]={"Packet32"};
constexpr char pipelineImgfusionName[]={"imgfusion"};
constexpr char pipelineRxifName[]={"PYTHON1300_RXIF"};

/* max width for roi end */
#define WIDTH 1280
#define HEIGHT 1024

#define NUM_BUF 8

enum io_method
{
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

struct buffer
{
        void *start;
        size_t length;
};

static int pipe_camera = 0; /* 1 for pipeline with real camera, 0 for dummy pipeline */
static char *media_name;
static int pipelineSubdeviceFd_[PIPELINE_MAX_LEN] = {
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
};
static int yuv = 0;
static int mainSubdeviceFd_ = -1;
static int sourceSubDeviceIndex1_ = -1;
static int sourceSubDeviceIndex2_ = -1;
static int rxif1_idx = -1;
static int rxif2_idx = -1;
static int csc_idx = -1;
static int tpg_idx = -1;
static int imgfusion_idx = -1;
static int packet32_idx = -1;
static int crop_l, crop_t, crop_h, crop_w;
static int crop_enable;
static enum io_method io = IO_METHOD_MMAP;
static int fd = -1;
static int fd_tmp = -1;
struct buffer *buffers;
static unsigned int n_buffers;
static int out_buf;
static int force_format;
static int frame_count = -1;
static int network = 0;
static int stream_file = 0;
static int port = 0;
static char ip_addr[24];
static int ip_socket;
static int grey = 0;
static int crop_move = 0;
static int subsampling = 0;
static int gpio = 0;
static int gpio_fd;
std::ofstream fs("./log.log");
std::string methodName{"------------"};

static void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
        int r;

        do
        {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

static void open_pipeline(void)
{
        fs << "open_pipeline" << methodName << std::endl;

        //Open main device
        int fd = open(media_name, O_RDWR);
        if (fd == -1)
        {
                fs << "ERROR-cannot open media dev" << std::endl;
                exit(EXIT_FAILURE);
        }
        fs << "open:" << media_name << " fd:" << fd << std::endl;

        struct udev *udev;
        udev = udev_new();
        if (udev == NULL)
        {
                fs << "ERROR-cannot open udev" << std::endl;
                exit(EXIT_FAILURE);
        }

        struct media_entity_desc info;
        int subdeviceIndex = 0;
        for (int id = 0;; id = info.id)
        {
                memset(&info, 0, sizeof(info));
                info.id = id | MEDIA_ENT_ID_FLAG_NEXT;

                int ret = ioctl(fd, MEDIA_IOC_ENUM_ENTITIES, &info);
                if (ret < 0)
                {
                        ret = errno != EINVAL ? -errno : 0;
                        fs << "WARNING-cannot open device not media" << std::endl;
                        break;
                }
                fs << "found entity num:" << id << " name:" << info.name << std::endl;

                dev_t devnum = makedev(info.v4l.major, info.v4l.minor);
                struct udev_device *device;
                device = udev_device_new_from_devnum(udev, 'c', devnum);
                if (device == nullptr)
                {
                        udev_device_unref(device);
                        continue;
                }

                const char *deviceName;
                deviceName = udev_device_get_devnode(device);

                //Open main subdevice
                if ((str::strcmp(info.name, pipelineVideoName) == 0) ||
                    (str::strcmp(info.name, pipelineDummyName) == 0))
                {
                        mainSubdeviceFd_ = open(deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);
                        if (mainSubdeviceFd_ == -1)
                        {
                                fs << "ERROR-cannot open device:" << mainSubdeviceFd_ << std::endl;
                                exit(EXIT_FAILURE);
                        }
                        fs << "open no pipeline:" << deviceName << " fd:" << mainSubdeviceFd_ << " device number:" << devnum << std::endl;
                }
                else
                {
                        //Open other subdevice

                        /*
				 * If a python camera is found in pipeline, then that's the
				 * source. If only a TPG is present, then it's the source.
				 * In case both are found, stick to camera
				 */
                        if (str::strcmp(info.name, pipelinePythonName) == 0)
                        {
                                if (sourceSubDeviceIndex1_ == -1)
                                        sourceSubDeviceIndex1_ = subdeviceIndex;
                                else
                                        sourceSubDeviceIndex2_ = subdeviceIndex;
                                pipe_camera = 1;
                        }
                        else if (std::strstr(info.name,pipelineTpgName))
                        {
                                tpg_idx = subdeviceIndex;
                        }
                        else if (std::strstr(info.name, pipelineCscName))
                        {
                                csc_idx = subdeviceIndex;
                        }
                        else if (std::strstr(info.name, pipelineImgfusionName))
                        {
                                imgfusion_idx = subdeviceIndex;
                        }
                        else if (std::strstr(info.name, pipelinePacket32Name))
                        {
                                packet32_idx = subdeviceIndex;
                        }
                        else if (std::strcmp(info.name, pipelineRxifName)==0)
                        {
                                if (rxif1_idx == -1)
                                        rxif1_idx = subdeviceIndex;
                                else
                                        rxif2_idx = subdeviceIndex;
                        }
                        pipelineSubdeviceFd_[subdeviceIndex] = open(deviceName, O_RDWR /* required */ | O_NONBLOCK, 0);
                        if (pipelineSubdeviceFd_[subdeviceIndex] == -1)
                        {
                                fs << "ERROR-cannot open device:" << deviceName << std::endl;
                                exit(EXIT_FAILURE);
                        }
                        fs << "open pipeline:" << deviceName << " fd:" << pipelineSubdeviceFd_[subdeviceIndex] << " device number:" << devnum << std::endl;
                        subdeviceIndex++;
                }

                udev_device_unref(device);
        }
        if (mainSubdeviceFd_ == -1)
        {
                fs << "ERROR-Cannot find main pipe V4L2 device" << std::endl;
                exit(EXIT_FAILURE);
        }
        if (sourceSubDeviceIndex1_ == -1)
        {
                fs << "ERROR-Cannot find source subdev" << std::endl;
                exit(EXIT_FAILURE);
        }

        fs << "final fd:" << mainSubdeviceFd_ << std::endl;
}

static void process_image(const void *p, int size)
{
        int ret;
        int i;
        int val = 1;
        uint8_t *ptr;

        if (out_buf)
                fwrite(p, size, 1, stdout);
        if (stream_file)
                write(fd_tmp, p, size);

        if (network)
        {
                ret = send(ip_socket, p, size, MSG_NOSIGNAL);
                if (ret != size)
                        fprintf(stderr, "Send failed with err %d -- errno: %d\n", ret, errno);
        }

        fflush(stderr);
        fprintf(stderr, ".");
        fflush(stdout);
}

static int read_frame(void)
{
        struct v4l2_buffer buf;
        unsigned int i;
        int seq = 1;
        static unsigned char dbg = 0;

        switch (io)
        {
        case IO_METHOD_READ:
                if (-1 == read(mainSubdeviceFd_, buffers[0].start, buffers[0].length))
                {
                        switch (errno)
                        {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("read");
                        }
                }

                process_image(buffers[0].start, buffers[0].length);
                break;

        case IO_METHOD_MMAP:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                //	usleep(5000);
                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_DQBUF, &buf))
                {
                        switch (errno)
                        {
                        case EAGAIN:
                                errno_exit("VIDIOC_DQBUF eagain");
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("VIDIOC_DQBUF");
                        }
                }

                assert(buf.index < n_buffers);
                if (buf.flags & V4L2_BUF_FLAG_ERROR)
                        errno_exit("V4L2_BUF_FLAG_ERROR");

                seq = buf.sequence;
                process_image(buffers[buf.index].start, buf.bytesused);
                memset(buffers[buf.index].start, dbg++, buf.bytesused);

                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                break;

        case IO_METHOD_USERPTR:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;

                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_DQBUF, &buf))
                {
                        switch (errno)
                        {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("VIDIOC_DQBUF");
                        }
                }

                for (i = 0; i < n_buffers; ++i)
                        if (buf.m.userptr == (unsigned long)buffers[i].start && buf.length == buffers[i].length)
                                break;

                assert(i < n_buffers);

                process_image((void *)buf.m.userptr, buf.bytesused);

                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                break;
        }

        return seq;
}

unsigned long sub_time_ms(struct timeval *time1, struct timeval *time2)
{
        struct timeval res;

        timersub(time1, time2, &res);
        return res.tv_sec * 1000 + res.tv_usec / 1000;
}

static void crop(int top, int left, int w, int h, int mytry);
static void mainloop(void)
{
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
        while (count != 0)
        {
                if (count > 0)
                        count--;
                for (;;)
                {
                        fd_set fds;
                        struct timeval tv;
                        int r;

                        FD_ZERO(&fds);
                        FD_SET(mainSubdeviceFd_, &fds);

                        /* Timeout. */
                        tv.tv_sec = 80;
                        tv.tv_usec = 0;

                        r = select(mainSubdeviceFd_ + 1,
                                   &fds, NULL, NULL, &tv);

                        if (-1 == r)
                        {
                                if (EINTR == errno)
                                        continue;
                                errno_exit("select");
                        }

                        if (0 == r)
                        {
                                fprintf(stderr, "select timeout\\n");
                                exit(EXIT_FAILURE);
                        }

                        if (seq = read_frame())
                        {
                                frames++;
                                if (io == IO_METHOD_MMAP && seq != sequence++)
                                {
                                        printf("dropped frame..\n");
                                        sequence = seq + 1;
                                }
                                gettimeofday(&time2, NULL);
                                time_delta = sub_time_ms(&time2, &time1);
                                if (time_delta >= 1000)
                                {
                                        fprintf(stderr, "fps: %f\n",
                                                ((double)frames / (double)time_delta) * 1000.0);
                                        time1 = time2;
                                        frames = 0;
                                }
                                break;
                        }
                        /* EAGAIN - continue select loop. */
                }

                if (crop_move)
                {
                        update_roi = 0;

                        if (left + crop_w > (WIDTH - 16))
                        {
                                roi_direction_h = -16;
                                left = WIDTH - crop_w - 16;
                        }
                        else if (left <= 0)
                        {
                                roi_direction_h = 16;
                                left = 0;
                        }
                        left += roi_direction_h;

                        if (top + crop_h > (HEIGHT - 16))
                        {
                                roi_direction_v = -1;
                                top = HEIGHT - crop_h - 16;
                        }
                        else if (top <= 0)
                        {
                                roi_direction_v = 1;
                                top = 0;
                        }
                        top += roi_direction_v;

                        crop(top, left, crop_w, crop_h, 0);
                }
        }
}

static void crop(int top, int left, int w, int h, int mytry)
{
        struct v4l2_subdev_crop _crop;

        _crop.rect.left = left;
        _crop.rect.top = top;
        _crop.rect.width = w;
        _crop.rect.height = h;

        _crop.which = mytry ? V4L2_SUBDEV_FORMAT_TRY : V4L2_SUBDEV_FORMAT_ACTIVE;
        _crop.pad = 0;

        //	printf("Crop enabled %d %d %d %d, %s\n",
        //	       top, left, w, h, mytry? "TRY" : "ACTIVE");

        if (-1 == xioctl(pipelineSubdeviceFd_[sourceSubDeviceIndex1_], VIDIOC_SUBDEV_S_CROP, &_crop))
                errno_exit("VIDIOC_SUBDEV_S_CROP");
        if (sourceSubDeviceIndex2_ != -1)
        {
                if (-1 == xioctl(pipelineSubdeviceFd_[sourceSubDeviceIndex2_], VIDIOC_SUBDEV_S_CROP, &_crop))
                        errno_exit("VIDIOC_SUBDEV_S_CROP");
        }
}

static void set_subsampling(void)
{
        struct v4l2_control ctrl;

        ctrl.id = V4L2_CID_XILINX_PYTHON1300_SUBSAMPLING;
        ctrl.value = !!subsampling;
        if (-1 == xioctl(pipelineSubdeviceFd_[sourceSubDeviceIndex1_], VIDIOC_S_CTRL, &ctrl))
                errno_exit("VIDIOC_S_CTRL subsampling");

        if (sourceSubDeviceIndex2_ != -1)
        {
                if (-1 == xioctl(pipelineSubdeviceFd_[sourceSubDeviceIndex2_], VIDIOC_S_CTRL, &ctrl))
                        errno_exit("VIDIOC_S_CTRL subsampling");
        }

        ctrl.id = V4L2_CID_XILINX_PYTHON1300_RXIF_REMAPPER_MODE;
        ctrl.value = subsampling ? 1 : 0;
        if (-1 == xioctl(pipelineSubdeviceFd_[rxif1_idx], VIDIOC_S_CTRL, &ctrl))
                errno_exit("VIDIOC_S_CTRL remapper");
        if (rxif2_idx != -1)
        {
                if (-1 == xioctl(pipelineSubdeviceFd_[rxif2_idx], VIDIOC_S_CTRL, &ctrl))
                        errno_exit("VIDIOC_S_CTRL remapper");
        }
}

static void stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (io)
        {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_STREAMOFF, &type))
                        errno_exit("VIDIOC_STREAMOFF");
                break;
        }
}

static void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (io)
        {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i)
                {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i)
                {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_USERPTR;
                        buf.index = i;
                        buf.m.userptr = (unsigned long)buffers[i].start;
                        buf.length = buffers[i].length;

                        if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;
        }
}

static void uninit_device(void)
{
        fs << "uninit_device" << methodName << std::endl;
        unsigned int i;

        switch (io)
        {
        case IO_METHOD_READ:
                free(buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i)
                        if (-1 == munmap(buffers[i].start, buffers[i].length))
                                errno_exit("munmap");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i)
                        free(buffers[i].start);
                break;
        }

        free(buffers);
}

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

static void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = NUM_BUF;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_REQBUFS, &req))
        {
                if (EINVAL == errno)
                {
                        fprintf(stderr, "device does not support memmap\n");
                        exit(EXIT_FAILURE);
                }
                else
                {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 1)
        {
                fprintf(stderr, "Insufficient buffer memory on \n");
                exit(EXIT_FAILURE);
        }

        buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));

        if (!buffers)
        {
                fprintf(stderr, "Out of memory\\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
        {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                buf.index = n_buffers;

                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                    mmap(NULL /* start anywhere */,
                         buf.length,
                         PROT_READ | PROT_WRITE /* required */,
                         MAP_SHARED /* recommended */,
                         mainSubdeviceFd_, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit("mmap");
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

static void subdevs_set_format(int width, int height)
{
        int i, j, n;
        struct v4l2_subdev_format fmt;
        char buf[256];

        for (i = 0; pipelineSubdeviceFd_[i] != -1; i++)
        {
                if (i == imgfusion_idx)
                        n = 3;
                else
                        n = 2;
                for (j = 0; j < n; j++)
                {
                        CLEAR(fmt);
                        fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;
                        fmt.pad = j;
                        if (-1 == xioctl(pipelineSubdeviceFd_[i], VIDIOC_SUBDEV_G_FMT, &fmt))
                        {
                                sprintf(buf, "VIDIOC_SUBDEV_G_FMT. subdev %d, pad %d",
                                        i, j);
                                errno_exit(buf);
                        }

                        fmt.format.width = width;
                        fmt.format.height = height;

                        /* if yuv is required, then set that on the source PAD of VPSS */
                        if ((i == csc_idx) && (j == 1) && yuv)
                        {
                                fmt.format.code = MEDIA_BUS_FMT_UYVY8_1X16;
                        }

                        /* csc, when there is an imgfusion IP receives 2x width frames */
                        if ((imgfusion_idx != -1) && (i == csc_idx))
                                fmt.format.width *= 2;
                        /* packet32, when there is an imgfusion IP receives 2x width frames */
                        if ((imgfusion_idx != -1) && (i == packet32_idx))
                                fmt.format.width *= 2;
                        /* tpg when there is an imgfusion IP receives 2x width frames */
                        if ((imgfusion_idx != -1) && (i == tpg_idx))
                                fmt.format.width *= 2;

                        /* imgfusion source pad has 2* width */
                        if (j == 2)
                                fmt.format.width *= 2;

                        fprintf(stderr, "subdev idx:%d, pad %d, setting format %dx%d\n",
                                i, j, fmt.format.width, fmt.format.height);

                        if (-1 == xioctl(pipelineSubdeviceFd_[i], VIDIOC_SUBDEV_S_FMT, &fmt))
                        {
                                sprintf(buf, "VIDIOC_SUBDEV_S_FMT. subdev %d, pad %d",
                                        i, j);
                                errno_exit(buf);
                        }
                        if ((i == sourceSubDeviceIndex1_) || (i == sourceSubDeviceIndex2_))
                                break; /* only one pad */
                }
        }
}

static void init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop _crop;
        struct v4l2_format fmt;
        unsigned int min;
        int i;

        if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_QUERYCAP, &cap))
        {
                if (EINVAL == errno)
                {
                        fprintf(stderr, "device is no V4L2 device\n");
                        exit(EXIT_FAILURE);
                }
                else
                {
                        errno_exit("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
                fprintf(stderr, "device is no video capture device\n");
                exit(EXIT_FAILURE);
        }

        switch (io)
        {
        case IO_METHOD_READ:
                if (!(cap.capabilities & V4L2_CAP_READWRITE))
                {
                        fprintf(stderr, "device does not support read i/o\n");
                        exit(EXIT_FAILURE);
                }
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                if (!(cap.capabilities & V4L2_CAP_STREAMING))
                {
                        fprintf(stderr, "device does not support streaming i/o\n");
                        exit(EXIT_FAILURE);
                }
                break;
        }

        /* Select video input, video standard and tune here. */

        CLEAR(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl(mainSubdeviceFd_, VIDIOC_CROPCAP, &cropcap))
        {
                _crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                _crop.c = cropcap.defrect; /* reset to default */

                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_S_CROP, &_crop))
                {
                        switch (errno)
                        {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        }
        else
        {
                /* Errors ignored. */
        }

        if (subsampling && !pipe_camera)
        {
                printf("subsampling is not supported with dummy pipe");
        }

        if (pipe_camera)
        {
                printf("subsampling is %s\n", subsampling ? "ENABLED" : "DISABLED");
                set_subsampling();
        }

        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (force_format || crop_enable)
        {
                fmt.fmt.pix.width = crop_enable ? crop_w : WIDTH;
                fmt.fmt.pix.height = crop_enable ? crop_h : HEIGHT;

                if (subsampling)
                {
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

                subdevs_set_format(fmt.fmt.pix.width, fmt.fmt.pix.height);

                if (imgfusion_idx != -1)
                        fmt.fmt.pix.width *= 2;

                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_S_FMT, &fmt))
                        errno_exit("VIDIOC_S_FMT");

                /* Note VIDIOC_S_FMT may change width and height. */
        }
        else
        {
                /* Preserve original settings as set by v4l2-ctl for example */
                if (-1 == xioctl(mainSubdeviceFd_, VIDIOC_G_FMT, &fmt))
                        errno_exit("VIDIOC_G_FMT");
        }

        if (crop_enable && !pipe_camera)
        {
                crop_enable = 0;
                printf("crop is not supported on dummy device\n");
        }

        printf("crop is %s\n", crop_enable ? "ENABLED" : "DISABLED");
        if (crop_enable)
                crop(crop_t, crop_l, crop_w, crop_h, 0);

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        switch (io)
        {
        case IO_METHOD_READ:
                init_read(fmt.fmt.pix.sizeimage);
                break;

        case IO_METHOD_MMAP:
                init_mmap();
                break;

        case IO_METHOD_USERPTR:
                init_userp(fmt.fmt.pix.sizeimage);
                break;
        }
}

static void open_socket(void)
{
        struct sockaddr_in addr;
        int ret;

        if (!network)
                return;

        memset(&addr, 0, sizeof(addr));

        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip_addr);
        addr.sin_family = AF_INET;

        ip_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (ip_socket == -1)
        {
                fprintf(stderr, "Cannot open socket: error %d\n", errno);
                exit(EXIT_FAILURE);
        }
        ret = connect(ip_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr));
        fprintf(stderr, "connect ret: %d\n", ret);
        if (ret == -1)
        {
                fprintf(stderr, "Cannot connect socket: error %d\n", errno);
                close(ip_socket);
                exit(EXIT_FAILURE);
        }
}

static void close_socket(void)
{
        if (!network)
                return;
        close(ip_socket);
}

static void close_pipeline(void)
{
        int i;

        for (i = 0; pipelineSubdeviceFd_[i] != -1; i++)
                if (-1 == close(pipelineSubdeviceFd_[i]))
                        errno_exit("close");
}

static void usage(FILE *fp, int argc, char **argv)
{
        fprintf(fp,
                "Usage: %s [options]\n\n"
                "Version 1.3\n"
                "Options:\n"
                "-d | --device <name>    MEDIA device name [%s]\n"
                "-h | --help             Print this message\n"
                "-m | --mmap             Use memory mapped buffers [default]\n"
                "-r | --read             Use read() calls\n"
                "-u | --userp            Use application allocated buffers\n"
                "-o | --output           Outputs stream to stdout\n"
                "-f | --format           Force format\n"
                "-y | --yuv              use VPSS to set YUV422 output\n"
                "-c | --count            Number of frames to grab [%i]\n"
                "-n | --net <ip:port>    Stream on TCP socket\n"
                "-g | --grey             BAYER format\n"
                "-p | --crop             crop [top,left,w,h]"
                "-v | --move             move crop around"
                "-t | --temp             stream in /run/tmpdat"
                "-b | --subsampling      enable subsampling"
                "-g | --gpio             check for center half-level light, write 1/0 in 'value' file",
                argv[0], media_name, frame_count);
}

static const char short_options[] = "d:hmruofc:n:gp:vtbiy";

static const struct option
    long_options[] = {
        {"device", required_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {"mmap", no_argument, NULL, 'm'},
        {"read", no_argument, NULL, 'r'},
        {"userp", no_argument, NULL, 'u'},
        {"output", no_argument, NULL, 'o'},
        {"format", no_argument, NULL, 'f'},
        {"count", required_argument, NULL, 'c'},
        {"grey", no_argument, NULL, 'g'},
        {"net", required_argument, NULL, 'n'},
        {"crop", required_argument, NULL, 'p'},
        {"move crop", no_argument, NULL, 'v'},
        {"temp file", no_argument, NULL, 't'},
        {"subsampling", no_argument, NULL, 'b'},
        {"gpio", no_argument, NULL, 'i'},
        {"yuv", no_argument, NULL, 'y'},
        {0, 0, 0, 0}};

int main(int argc, char **argv)
{
        fs << "main" << methodName << std::endl;

        int ret;
        char buf[1024];
        int i;
        media_name = "/dev/video0";
        //subdev_name = "/dev/v4l-subdev1-foo";

        for (;;)
        {
                int idx;
                int c;

                c = getopt_long(argc, argv,
                                short_options, long_options, &idx);

                if (-1 == c)
                        break;

                switch (c)
                {
                case 0: /* getopt_long() flag */
                        break;
                case 'v':
                        crop_move = 1;
                        break;

                case 'p':
                        sscanf(optarg, "%d,%d,%d,%d", &crop_t, &crop_l, &crop_w, &crop_h);
                        crop_enable = 1;
                        break;

                case 'g':
                        grey = 1;
                        break;
                case 'd':
                        media_name = optarg;
                        break;

                case 'h':
                        usage(stdout, argc, argv);
                        exit(EXIT_SUCCESS);

                case 'm':
                        io = IO_METHOD_MMAP;
                        break;

                case 'r':
                        io = IO_METHOD_READ;
                        break;

                case 'u':
                        io = IO_METHOD_USERPTR;
                        break;

                case 'o':
                        out_buf++;
                        break;

                case 'f':
                        force_format++;
                        break;

                case 'c':
                        errno = 0;
                        frame_count = strtol(optarg, NULL, 0);
                        if (errno)
                                errno_exit(optarg);
                        break;
                case 'n':
                        network = 1;
                        for (i = 0; i < strlen(optarg); i++)
                                if (optarg[i] == ':')
                                        break;
                        sscanf(optarg + i, ":%d", &port);
                        optarg[i] = '\0';
                        strcpy(ip_addr, optarg);
                        break;
                case 't':
                        stream_file = 1;
                        break;

                case 'b':
                        subsampling = 1;
                        break;

                case 'i':
                        gpio = 1;
                        break;
                case 'y':
                        yuv = 1;
                        break;

                default:
                        usage(stderr, argc, argv);
                        exit(EXIT_FAILURE);
                }
        }

        if (crop_move && !crop_enable)
        {
                printf("crop not enabled, ignoring move crop\n");
                crop_move = 0;
        }

        open_pipeline();
        init_device();
        fd_tmp = open("/run/tmpdat", O_WRONLY | O_CREAT);
        if (gpio)
                gpio_fd = open("value", O_WRONLY);
        if (gpio_fd < 0)
        {
                fprintf(stderr, "--gpio option specified, but cannot open 'value' file. Disabling\n");
                gpio = 0;
        }
        open_socket();
        start_capturing();
        mainloop();
        stop_capturing();
        uninit_device();
        close_pipeline();
        close_socket();
        close(fd_tmp);
        fprintf(stderr, "\\n");
        return 0;
}
