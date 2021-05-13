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
#include <yarp/os/Semaphore.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <opencv2/opencv.hpp>

#include "UltraPythonCameraHelper.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

class V4L_camera : public yarp::dev::DeviceDriver, public yarp::dev::IFrameGrabberRgb, public yarp::dev::IFrameGrabberControls,  public yarp::dev::IRgbVisualParams
{
   public:
	V4L_camera();

	// DeviceDriver Interface
	bool open(yarp::os::Searchable &config) override;
	bool close() override;

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

	yarp::os::Semaphore mutex;
	bool configured_{false};

	bool fromConfig(yarp::os::Searchable &config);

   private:

	UltraPythonCameraHelper pythonCameraHelper_;
	void pythonPreprocess(const void *pythonbuffer, size_t size);
};
