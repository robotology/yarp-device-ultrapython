/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see https://linuxtv.org/docs.php for more information
 */
typedef unsigned long size_t;
#include <array>
#include <cstring>
#include <fstream>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>
#include <string.h>

#include <getopt.h> /* getopt_long() */

#include "xilinx-v4l2-controls.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h> /* low-level i/o */
#include <libudev.h>
#include <linux/media.h>
#include <linux/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "PythonCameraHelper.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

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


static void process_image(const void *p, int size) {
  int ret;
  int i;
  int val = 1;
  uint8_t *ptr;

  if (p == nullptr) {
    pythonHelper.fs << "ERROR-nullptr process_image" << std::endl;
    return;
  }

  if (out_buf)
    fwrite(p, size, 1, stdout);
  if (stream_file)
    write(fd_tmp, p, size);

  if (network) {
    ret = send(ip_socket, p, size, MSG_NOSIGNAL);
    if (ret != size)
      fprintf(stderr, "Send failed with err %d -- errno: %d\n", ret, errno);
  }

  fflush(stderr);
  fprintf(stderr, ".");
  fflush(stdout);
}

static void open_socket(void) {
  struct sockaddr_in addr;
  int ret;

  if (!network)
    return;

  memset(&addr, 0, sizeof(addr));

  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip_addr);
  addr.sin_family = AF_INET;

  ip_socket = socket(AF_INET, SOCK_STREAM, 0);

  if (ip_socket == -1) {
    fprintf(stderr, "Cannot open socket: error %d\n", errno);
    exit(EXIT_FAILURE);
  }
  ret = connect(ip_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr));
  fprintf(stderr, "connect ret: %d\n", ret);
  if (ret == -1) {
    fprintf(stderr, "Cannot connect socket: error %d\n", errno);
    close(ip_socket);
    exit(EXIT_FAILURE);
  }
}

static void close_socket(void) {
  if (!network)
    return;
  close(ip_socket);
}

static void usage(FILE *fp, int argc, char **argv) {
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
          "-g | --gpio             check for center half-level light, write "
          "1/0 in 'value' file",
          argv[0], pythonHelper.mediaName_, frame_count);
}

static const char short_options[] = "d:hmruofc:n:gp:vtbiy";

static const struct option long_options[] = {
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

int main(int argc, char **argv) {

  int ret;
  char buf[1024];
  int i;

  for (;;) {
    int idx;
    int c;

    c = getopt_long(argc, argv, short_options, long_options, &idx);

    if (-1 == c)
      break;

    switch (c) {
    case 'p':
      sscanf(optarg, "%d,%d,%d,%d", &pythonHelper.cropTop_,
             &pythonHelper.cropLeft_, &pythonHelper.cropWidth_,
             &pythonHelper.cropHeight_);
      pythonHelper.cropEnabledProperty_ = true;
      break;

    case 'g':
      pythonHelper.spaceColor_ =SpaceColor::grgb;
    
      break;
    case 'd':
      pythonHelper.mediaName_ = optarg;
      break;

    case 'h':
      usage(stdout, argc, argv);
      exit(EXIT_SUCCESS);

    case 'm':
    case 'r':
    case 'u':
    case 'v':
    case '0':
    case 'i':
      break;

    case 'o':
      out_buf++;
      break;

    case 'f':
      pythonHelper.forceFormatProperty_ = true;
      pythonHelper.fs << "SETINGS-forceFormatProperty_"
                      << pythonHelper.forceFormatProperty_ << std::endl;
      break;

    case 'c':
      errno = 0;
      frame_count = strtol(optarg, NULL, 0);
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
      break;
    case 'y':
      pythonHelper.spaceColor_ =SpaceColor::yuv;
      break;

    default:
      usage(stderr, argc, argv);
      exit(EXIT_FAILURE);
    }
  }

  pythonHelper.injectedProcessImage_ = process_image;
  fd_tmp = open("/run/tmpdat", O_WRONLY | O_CREAT);

  pythonHelper.openPipeline();
  pythonHelper.initDevice();
  open_socket();
  pythonHelper.startCapturing();
  while (true)
    pythonHelper.mainLoop();
  pythonHelper.closeAll();
  close_socket();
  close(fd_tmp);
  return 0;
}
