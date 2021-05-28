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

#include "UltraPythonDriver.h"

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/FrameGrabberInterfaces.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>

#include "InterfaceForCApi.h"
#include "common.h"

using namespace yarp::os;
using namespace yarp::dev;

YARP_LOG_COMPONENT(ULTRAPYTHON, "yarp.device.UltraPython")

UltraPythonDriver::UltraPythonDriver() : pythonCameraHelper_(nullptr)
{
	yCTrace(ULTRAPYTHON) << "---------------------------------------------";
	yCTrace(ULTRAPYTHON) << "------UltraPython device ready to start------";
	yCTrace(ULTRAPYTHON) << "---------------------------------------------";

	pythonCameraHelper_.setInjectedProcess([this](const void *pythonBuffer, size_t size) { pythonPreprocess(pythonBuffer, size); });
	pythonCameraHelper_.setInjectedUnlock([this]() { mutex.post(); });
	pythonCameraHelper_.setInjectedLock([this]() { mutex.wait(); });
	pythonCameraHelper_.setInjectedLog([](const std::string &toLog, Severity severity) {
		switch (severity)
		{
			case Severity::error:
				yCError(ULTRAPYTHON) << toLog;
				break;
			case Severity::info:
				yCInfo(ULTRAPYTHON) << toLog;
				break;
			case Severity::debug:
				yCDebug(ULTRAPYTHON) << toLog;
				break;
			case Severity::warning:
				yCWarning(ULTRAPYTHON) << toLog;
				break;
		}
	});
}

UltraPythonDriver::~UltraPythonDriver()
{
	yCTrace(ULTRAPYTHON) << "Destroy drive";
}

bool UltraPythonDriver::open(yarp::os::Searchable &config)
{
	yarp::os::Property prop;
	prop.fromString(config.toString());

	yCTrace(ULTRAPYTHON) << "input params are " << config.toString();

	if (!fromConfig(config))
	{
		return false;
	}

	if (!pythonCameraHelper_.openAll())
		return false;
	configured_ = true;
	yarp::os::Time::delay(0.5);
	return true;
}

bool UltraPythonDriver::close()
{
	yCTrace(ULTRAPYTHON);

	return pythonCameraHelper_.closeAll();
}

int UltraPythonDriver::width() const
{
	return pythonCameraHelper_.currentWidth_;
}

int UltraPythonDriver::height() const
{
	return pythonCameraHelper_.currentHeight_;
}

yarp::os::Stamp UltraPythonDriver::getLastInputStamp()
{
	// TODO
	return yarp::os::Stamp();
}

bool UltraPythonDriver::getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb> &image)
{
	if (!configured_)
	{
		yCError(ULTRAPYTHON) << "unable to get the buffer, device uninitialized";
		return false;
	}

	if ((image.width() != width()) || (image.height() != height()))
	{
		image.resize(width(), height());
	}

	mutex.wait();
	if (pythonCameraHelper_.step(image.getRawImage()))
	{
	}
	else
	{
		yCError(ULTRAPYTHON) << "Failed acquiring new frame";
	}
	mutex.post();
	return true;
}

bool UltraPythonDriver::getCameraDescription(CameraDescriptor *camera)
{
	camera->busType = BUS_UNKNOWN;
	camera->deviceDescription = "UltraPython camera";
	return true;
}

bool UltraPythonDriver::hasFeature(int feature, bool *_hasFeature)
{
	if (!FeatureHelper::exists(feature))
	{
		yCError(ULTRAPYTHON) << "This feature is not supported:" << feature;
		return false;
	}
	return true;
}

bool UltraPythonDriver::setFeature(int feature, double value)
{
	if (!FeatureHelper::existsForWrite(feature))
	{
		yCError(ULTRAPYTHON) << "This feature is not for write:" << feature;
		return false;
	}

	bool absolute{false};
	if (FeatureHelper::isAbsolute(feature))
	{
		absolute = true;
	}
	bool ret = pythonCameraHelper_.setControl(remapControlYARPtoXilinx(feature), value, absolute);
	return ret;
}

bool UltraPythonDriver::getFeature(int feature, double *value)
{
	if (!FeatureHelper::existsForRead(feature))
	{
		yCError(ULTRAPYTHON) << "This feature is not for read:" << feature;
		return false;
	}

	bool absolute{false};
	if (FeatureHelper::isAbsolute(feature))
	{
		absolute = true;
	}

	double tmp = 0.0;
	tmp = pythonCameraHelper_.getControl(remapControlYARPtoXilinx(feature), absolute);
	if (tmp == -1)
	{
		*value = 0;
		return false;
	}

	*value = tmp;

	yCDebug(ULTRAPYTHON) << "getControl feature:" << feature<<" value:"<<*value;
	return true;
}

bool UltraPythonDriver::getFeature(int feature, double *value1, double *value2)
{
	yCError(ULTRAPYTHON) << "getFeature double param - not supported";
	return false;
}

bool UltraPythonDriver::setFeature(int feature, double value1, double value2)
{
	yCError(ULTRAPYTHON) << "setFeature double param - not supported";
	return false;
}

bool UltraPythonDriver::hasOnOff(int feature, bool *_hasOnOff)
{
	*_hasOnOff = false;
	return true;
}

bool UltraPythonDriver::getActive(int feature, bool *_isActive)
{
	*_isActive = true;
	return true;
}

bool UltraPythonDriver::hasAuto(int feature, bool *_hasAuto)
{
	*_hasAuto = false;
	return true;
}

bool UltraPythonDriver::hasManual(int feature, bool *_hasManual)
{
	*_hasManual = false;
	if (FeatureHelper::exists(feature))
		*_hasManual = true;
	return true;
}

bool UltraPythonDriver::setMode(int feature, FeatureMode mode)
{
	yCError(ULTRAPYTHON) << "Feature " << feature << " does not support auto mode";
	return false;
}

bool UltraPythonDriver::getMode(int feature, FeatureMode *mode)
{
	*mode = MODE_MANUAL;
	return true;
}

bool UltraPythonDriver::setActive(int feature, bool onoff)
{
	yCError(ULTRAPYTHON) << "setActive - not supported";
	return false;
}

bool UltraPythonDriver::hasOnePush(int feature, bool *_hasOnePush)
{
	yCError(ULTRAPYTHON) << "hasOnePush - not supported";
	*_hasOnePush = false;
	return false;
}

bool UltraPythonDriver::setOnePush(int feature)
{
	yCError(ULTRAPYTHON) << "setOnePush - not supported";
	return false;
}

bool UltraPythonDriver::fromConfig(yarp::os::Searchable &config)
{
	if (config.check("verbose"))
	{
		// TODO
	}

	int period = 28;
	if (config.check("period"))
	{
		auto tmp = config.find("period");
		period = tmp.asInt32();
		yCInfo(ULTRAPYTHON) << "Period used:" << period;
		pythonCameraHelper_.setStepPeriod(period);	// For exposition setting check
	}

	if (config.check("honorfps"))
	{
		bool honor;
		auto tmp = config.find("honorfps");
		honor = tmp.asBool();
		yCInfo(ULTRAPYTHON) << "HonorFps:" << honor;
		pythonCameraHelper_.setHonorFps(honor);
	}

	if (!config.check("subsampling"))
	{
		yCDebug(ULTRAPYTHON) << "Python cam full-sampling ";
		pythonCameraHelper_.setSubsamplingProperty(false);
		if (1000.0 / (double)period > UltraPythonCameraHelper::hiresFrameRate_)
		{
			yCWarning(ULTRAPYTHON) << "FPS exceed suggested FPS for hires:" << 1000.0 / (double)period << " suggested:" << UltraPythonCameraHelper::hiresFrameRate_;
		}
	}
	else
	{
		yCDebug(ULTRAPYTHON) << "Python cam sub-sampling ";
		pythonCameraHelper_.setSubsamplingProperty(true);
		if (1000.0 / (double)period > UltraPythonCameraHelper::lowresFrameRate_)
		{
			yCWarning(ULTRAPYTHON) << "FPS exceed suggested FPS for lowres:" << 1000.0 / (double)period << " suggested:" << UltraPythonCameraHelper::lowresFrameRate_;
		}
	}

	if (config.check("cammodel"))
	{
		yCError(ULTRAPYTHON) << "cammodel - param not supported.)";
	}

	if (config.check("d"))
	{
		yCError(ULTRAPYTHON) << "d - param not supported.)";
	}

	if (config.check("flip"))
	{
		yCError(ULTRAPYTHON, "flip - param not supported.");
	}

	if (config.check("crop"))
	{
		yCError(ULTRAPYTHON, "crop - param not supported.");
	}

	if (config.check("dual"))
	{
		yCError(ULTRAPYTHON, "dual - param not supported.");
	}

	if (config.check("framerate"))
	{
		yCWarning(ULTRAPYTHON) << "framerate - param not supported.";
	}

	if (config.check("pixelType"))
	{
		yCError(ULTRAPYTHON) << "pixelType - param not supported.";
	}

	if (config.check("horizontalFov"))
	{
		yCError(ULTRAPYTHON) << "horizontalFov - param not supported.";
	}

	if (config.check("verticalFov"))
	{
		yCError(ULTRAPYTHON) << "verticalFov - param not supported.";
	}
	if (config.check("principalPointX"))
	{
		yCError(ULTRAPYTHON) << "principalPointX - param not supported.";
	}
	if (config.check("principalPointY"))
	{
		yCError(ULTRAPYTHON) << "principalPointY - param not supported.";
	}
	if (config.check("retificationMatrix"))
	{
		yCError(ULTRAPYTHON) << "retificationMatrix - param not supported.";
	}
	if (config.check("distortionModel"))
	{
		yCError(ULTRAPYTHON) << "distortionModel - param not supported.";
	}

	yCDebug(ULTRAPYTHON) << "Ultrapython with the configuration: " << pythonCameraHelper_.currentWidth_ << "x" << pythonCameraHelper_.currentHeight_;
	return true;
}

void UltraPythonDriver::pythonPreprocess(const void *pythonbuffer, size_t size)
{
	// Nothing to do
}

int UltraPythonDriver::remapControlYARPtoXilinx(int feature) const
{
	switch (feature)
	{
		case YARP_FEATURE_BRIGHTNESS:
		case YARP_FEATURE_BRIGHTNESS_ABSOLUTE:
			yCDebug(ULTRAPYTHON) << "Feature " << feature << " does not support auto mode";
			return V4L2_CID_BRIGHTNESS;
		case YARP_FEATURE_SHUTTER:
		case YARP_FEATURE_EXPOSURE:			  // shutter used
		case YARP_FEATURE_EXPOSURE_ABSOLUTE:  // shutter used
			yCDebug(ULTRAPYTHON) << "remap exposure";
			return UltraPythonCameraHelper::V4L2_EXPOSURE_ULTRA_PYTHON;
		case YARP_FEATURE_GAIN:
		case YARP_FEATURE_GAIN_ABSOLUTE:
			yCDebug(ULTRAPYTHON) << "remap gain";
			return V4L2_CID_GAIN;
		case YARP_FEATURE_RED_GAIN:
		case YARP_FEATURE_RED_GAIN_ABSOLUTE:
			yCDebug(ULTRAPYTHON) << "remap RED gain";
			return UltraPythonCameraHelper::V4L2_REDBALANCE_ULTRA_PYTHON;
		case YARP_FEATURE_GREEN_GAIN:
		case YARP_FEATURE_GREEN_GAIN_ABSOLUTE:
			yCDebug(ULTRAPYTHON) << "remap GREEN gain";
			return UltraPythonCameraHelper::V4L2_GREENBALANCE_ULTRA_PYTHON;
		case YARP_FEATURE_BLUE_GAIN:
		case YARP_FEATURE_BLUE_GAIN_ABSOLUTE:
			yCDebug(ULTRAPYTHON) << "remap BLUE gain";
			return UltraPythonCameraHelper::V4L2_BLUEBALANCE_ULTRA_PYTHON;
	}
	yCDebug(ULTRAPYTHON) << "not remapped feature:" << feature;
	return feature;
}