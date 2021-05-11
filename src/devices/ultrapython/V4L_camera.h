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

#pragma once

#include <asm/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <getopt.h>
#include <jpeglib.h>
#include <libv4l2.h>
#include <libv4lconvert.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/FrameGrabberInterfaces.h>
#include <yarp/dev/IPreciselyTimed.h>
#include <yarp/dev/IVisualParams.h>
#include <yarp/os/PeriodicThread.h>
#include <yarp/os/Semaphore.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>

#include "UltraPythonCameraHelper.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

// minimum number of buffers to request in VIDIOC_REQBUFS call
#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
#define DEFAULT_FRAMERATE 30
#define VIDIOC_REQBUFS_COUNT 2

typedef enum
{
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;

typedef enum
{
	ULTRAPYTON
} supported_cams;

struct buffer
{
	void *start;
	size_t length;
};

typedef struct
{
	
	__u32 user_width;
	__u32 user_height;
		
	
		supported_cams camModel;  // In case some camera requires custom procedure
} Video_params;

/*
 *  Device handling
 */

class V4L_camera : public yarp::dev::DeviceDriver,
				   public yarp::dev::IFrameGrabberRgb,
				   public yarp::dev::IFrameGrabberControls,
				   public yarp::dev::IPreciselyTimed,
				   public yarp::dev::IRgbVisualParams
{
   public:
	V4L_camera();

	// DeviceDriver Interface
	bool open(yarp::os::Searchable &config) override;
	bool close() override;

// IPreciselyTimed    Interface
	yarp::os::Stamp getLastInputStamp() override;

	// IFrameGrabberRgb    Interface
	bool getRgbBuffer(unsigned char *buffer) override;
	int height() const override;
	int width() const override;

	/*Implementation of IRgbVisualParams interface*/
	int getRgbHeight() override;
	int getRgbWidth() override;
	bool getRgbSupportedConfigurations(yarp::sig::VectorOf<yarp::dev::CameraConfig> &configurations) override;
	bool getRgbResolution(int &width, int &height) override;
	bool setRgbResolution(int width, int height) override;
	bool getRgbFOV(double &horizontalFov, double &verticalFov) override;
	bool setRgbFOV(double horizontalFov, double verticalFov) override;
	bool getRgbIntrinsicParam(yarp::os::Property &intrinsic) override;
	bool getRgbMirroring(bool &mirror) override;
	bool setRgbMirroring(bool mirror) override;

	/* Implementation of IFrameGrabberControls interface */
	bool getCameraDescription(CameraDescriptor *camera) override;
	bool hasFeature(int feature, bool *hasFeature) override;
	bool setFeature(int feature, double value) override;
	bool getFeature(int feature, double *value) override;
	bool setFeature(int feature, double value1, double value2) override;
	bool getFeature(int feature, double *value1, double *value2) override;
	bool hasOnOff(int feature, bool *_hasOnOff) override;
	bool setActive(int feature, bool onoff) override;
	bool getActive(int feature, bool *_isActive) override;
	bool hasAuto(int feature, bool *_hasAuto) override;
	bool hasManual(int feature, bool *_hasManual) override;
	bool hasOnePush(int feature, bool *_hasOnePush) override;
	bool setMode(int feature, FeatureMode mode) override;
	bool getMode(int feature, FeatureMode *mode) override;
	bool setOnePush(int feature) override;

   private:
	bool verbose{false};
	v4lconvert_data *_v4lconvert_data;
	bool use_exposure_absolute;

	yarp::os::Stamp timeStamp;
	Video_params param;
	yarp::os::Semaphore mutex;
	bool configured;
	bool isActive_vector[YARP_FEATURE_NUMBER_OF];
	double timeStart, timeTot, timeNow, timeElapsed;

	bool fromConfig(yarp::os::Searchable &config);

	int convertV4L_to_YARP_format(int format);

	  private:

	int convertYARP_to_V4L(int feature);
	bool check_V4L2_control(uint32_t id);
	bool set_V4L2_control(u_int32_t id, double value, bool verbatim = false);
	double get_V4L2_control(uint32_t id,
							bool verbatim = false);	 // verbatim = do not convert value, for enum types

	// Only for PythonCamera
	UltraPythonCameraHelper pythonCameraHelper_;
	void pythonPreprocess(const void *pythonbuffer, size_t size);
	unsigned char *pythonBuffer_;
	unsigned int pythonBufferSize_{0};
};
