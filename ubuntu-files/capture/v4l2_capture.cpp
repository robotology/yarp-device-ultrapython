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
#include <array>
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

#include "PythonCameraHelper.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))



constexpr unsigned int pipelineMaxLen = {16};

PythonCameraHelper pythonHelper;

static int fd = -1;
static int fd_tmp = -1;

static int out_buf;
static int frame_count = -1;
static int network = 0;
static int stream_file = 0;
static int port = 0;
static char ip_addr[24];
static int ip_socket;
static int crop_move = 0;


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

        switch (pythonHelper.ioMethod_)
        {
        case IO_METHOD_READ:
                if (-1 == read(pythonHelper.mainSubdeviceFd_, pythonHelper.buffers[0].start, pythonHelper.buffers[0].length))
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

                process_image(pythonHelper.buffers[0].start, pythonHelper.buffers[0].length);
                break;

        case IO_METHOD_MMAP:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;
                //	usleep(5000);
                if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_DQBUF, &buf))
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

                assert(buf.index < pythonHelper.n_buffers);
                if (buf.flags & V4L2_BUF_FLAG_ERROR)
                        errno_exit("V4L2_BUF_FLAG_ERROR");

                seq = buf.sequence;
                process_image(pythonHelper.buffers[buf.index].start, buf.bytesused);
                memset(pythonHelper.buffers[buf.index].start, dbg++, buf.bytesused);

                if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                break;

        case IO_METHOD_USERPTR:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;

                if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_DQBUF, &buf))
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

                for (i = 0; i < pythonHelper.n_buffers; ++i)
                        if (buf.m.userptr == (unsigned long)pythonHelper.buffers[i].start && buf.length == pythonHelper.buffers[i].length)
                                break;

                assert(i < pythonHelper.n_buffers);

                process_image((void *)buf.m.userptr, buf.bytesused);

                if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_QBUF, &buf))
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
                        FD_SET(pythonHelper.mainSubdeviceFd_, &fds);

                        /* Timeout. */
                        tv.tv_sec = 80;
                        tv.tv_usec = 0;

                        r = select(pythonHelper.mainSubdeviceFd_ + 1,
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
                                if (pythonHelper.ioMethod_ == IO_METHOD_MMAP && seq != sequence++)
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

                        if (left + pythonHelper.cropWidth_ > (pythonHelper.nativeWidth_ - 16))
                        {
                                roi_direction_h = -16;
                                left = pythonHelper.nativeWidth_ - pythonHelper.cropWidth_ - 16;
                        }
                        else if (left <= 0)
                        {
                                roi_direction_h = 16;
                                left = 0;
                        }
                        left += roi_direction_h;

                        if (top + pythonHelper.cropHeight_ > (pythonHelper.nativeHeight_ - 16))
                        {
                                roi_direction_v = -1;
                                top = pythonHelper.nativeHeight_ - pythonHelper.cropHeight_ - 16;
                        }
                        else if (top <= 0)
                        {
                                roi_direction_v = 1;
                                top = 0;
                        }
                        top += roi_direction_v;

                        pythonHelper.crop(top, left, pythonHelper.cropWidth_, pythonHelper.cropHeight_, 0);
                }
        }
}

static void stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (pythonHelper.ioMethod_)
        {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_STREAMOFF, &type))
                        errno_exit("VIDIOC_STREAMOFF");
                break;
        }
}

static void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (pythonHelper.ioMethod_)
        {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < pythonHelper.n_buffers; ++i)
                {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < pythonHelper.n_buffers; ++i)
                {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_USERPTR;
                        buf.index = i;
                        buf.m.userptr = (unsigned long)pythonHelper.buffers[i].start;
                        buf.length = pythonHelper.buffers[i].length;

                        if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(pythonHelper.mainSubdeviceFd_, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;
        }
}

static void uninit_device(void)
{
        pythonHelper.fs << "uninit_device" << pythonHelper.methodName << std::endl;
        unsigned int i;

        switch (pythonHelper.ioMethod_)
        {
        case IO_METHOD_READ:
                free(pythonHelper.buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < pythonHelper.n_buffers; ++i)
                        if (-1 == munmap(pythonHelper.buffers[i].start, pythonHelper.buffers[i].length))
                                errno_exit("munmap");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < pythonHelper.n_buffers; ++i)
                        free(pythonHelper.buffers[i].start);
                break;
        }

        free(pythonHelper.buffers);
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

        for (i = 0; pythonHelper.pipelineSubdeviceFd_[i] != -1; i++)
                if (-1 == close(pythonHelper.pipelineSubdeviceFd_[i]))
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
                argv[0], pythonHelper.mediaName_, frame_count);
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
        pythonHelper.fs << "main" << pythonHelper.methodName << std::endl;

        int ret;
        char buf[1024];
        int i;
        pythonHelper.mediaName_ = "/dev/video0";
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
                        sscanf(optarg, "%d,%d,%d,%d", &pythonHelper.cropTop_, &pythonHelper.cropLeft_, &pythonHelper.cropWidth_, &pythonHelper.cropHeight_);
                        pythonHelper.cropEnabledProperty_ = true;
                        break;

                case 'g':
                        pythonHelper.grey = 1;
                        pythonHelper.fs << "SETINGS-grey" << pythonHelper.grey << std::endl;
                        break;
                case 'd':
                        pythonHelper.mediaName_ = optarg;
                        break;

                case 'h':
                        usage(stdout, argc, argv);
                        exit(EXIT_SUCCESS);

                case 'm':
                        pythonHelper.ioMethod_ = IO_METHOD_MMAP;
                        break;

                case 'r':
                        pythonHelper.ioMethod_ = IO_METHOD_READ;
                        break;

                case 'u':
                        pythonHelper.ioMethod_ = IO_METHOD_USERPTR;
                        break;

                case 'o':
                        out_buf++;
                        break;

                case 'f':
                        pythonHelper.forceFormatProperty_ = true;
                        pythonHelper.fs << "SETINGS-forceFormatProperty_" << pythonHelper.forceFormatProperty_ << std::endl;
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
                        pythonHelper.subsamplingEnabledProperty_ = true;
                        break;

                case 'i':
                      
                        break;
                case 'y':
                        pythonHelper.yuv = 1;
                        pythonHelper.fs << "SETINGS-yov" << pythonHelper.yuv << std::endl;
                        break;

                default:
                        usage(stderr, argc, argv);
                        exit(EXIT_FAILURE);
                }
        }

        if (crop_move && !pythonHelper.cropEnabledProperty_)
        {
                printf("crop not enabled, ignoring move crop\n");
                crop_move = 0;
        }

        pythonHelper.openPipeline();

        pythonHelper.initDevice();
        fd_tmp = open("/run/tmpdat", O_WRONLY | O_CREAT);
        
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
