#pragma once

#include <array>
#include <cstring>
#include <fstream>
#include <functional>

struct v4l2_format;

struct buffer {
  void *start;
  size_t length;
};

class PythonCameraHelper {
private:
  // Pipeline string
  inline static constexpr char pipelineVideoName[] = {"vcap_python output 0"};
  inline static constexpr char pipelineDummyName[] = {"vcap_dummy output 0"};
  inline static constexpr char pipelinePythonName[] = {"PYTHON1300"};
  inline static constexpr char pipelineTpgName[] = {"v_tpg"};
  inline static constexpr char pipelineCscName[] = {"v_proc_ss"};
  inline static constexpr char pipelinePacket32Name[] = {"Packet32"};
  inline static constexpr char pipelineImgfusionName[] = {"imgfusion"};
  inline static constexpr char pipelineRxifName[] = {"PYTHON1300_RXIF"};

  static constexpr unsigned int requestBufferNumber_ = {8};
  static constexpr unsigned int pipelineMaxLen = {16};

public:
  void openPipeline();
  void initDevice();
  void startCapturing();
  void mainLoop();
  void closeAll();

  bool subsamplingEnabledProperty_{false};
  bool cropEnabledProperty_{false};
  bool forceFormatProperty_{true};

  std::array<int, pipelineMaxLen> pipelineSubdeviceFd_ = {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  };

  int mainSubdeviceFd_ = -1;
  int sourceSubDeviceIndex1_ = -1;
  int sourceSubDeviceIndex2_ = -1;
  int rxif1Index_ = -1;
  int rxif2Index_ = -1;
  int cscIndex_ = -1;
  int tpgIndex_ = -1;
  int imgfusionIndex_ = -1;
  int packet32Index_ = -1;

  bool keepCapturing_{true};

  std::string mediaName_{"/dev/video0"};

  // Crop size
  unsigned int cropLeft_{0};
  unsigned int cropTop_{0};
  unsigned int cropHeight_{0};
  unsigned int cropWidth_{0};

  // Native resolution for cam
  inline static constexpr unsigned int nativeWidth_{1280};
  inline static constexpr unsigned int nativeHeight_{1024};

  // Process image external
  std::function<void(const void *, int)> injectedProcessImage_;
  // TODO change
  int grey = 0;
  int yuv = 0;
  struct buffer *buffers;

  std::ofstream fs{"./log.log"};
  inline static constexpr char *methodName[]{"------------"};

private:
  void setSubDevFormat(int width, int height);
  void setFormat();
  void setSubsampling(void);
  void crop(int top, int left, int w, int h, int mytry);
  bool checkDevice(int mainSubdeviceFd);
  int readFrame();
  void processImage(const void *p, int size);
  void unInitDevice(void);

  int xioctl(int fh, int request, void *arg);
  void initMmap(void);
  bool cropCheck();
  unsigned long subTimeMs(struct timeval *time1, struct timeval *time2);
};