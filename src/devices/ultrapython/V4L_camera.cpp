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

#include "V4L_camera.h"

#include <opencv2/core/core_c.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Time.h>
#include <yarp/os/Value.h>

#include <cstdio>
#include <ctime>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "InterfaceForCApi.h"
#include "Statistics.h"
#include "UltraPythonCameraLogComponent.h"
#include "list.h"

using namespace yarp::os;
using namespace yarp::dev;

#define NOT_PRESENT -1
int V4L_camera::convertYARP_to_V4L(int feature)
{
	switch (feature)
	{
		case YARP_FEATURE_BRIGHTNESS:
			return V4L2_CID_BRIGHTNESS;
		case YARP_FEATURE_SHUTTER:	// this maps also on exposure
		case YARP_FEATURE_EXPOSURE:
			return V4L2_CID_EXPOSURE;
		case YARP_FEATURE_SHARPNESS:
			return V4L2_CID_SHARPNESS;
		case YARP_FEATURE_HUE:
			return V4L2_CID_HUE;
		case YARP_FEATURE_SATURATION:
			return V4L2_CID_SATURATION;
		case YARP_FEATURE_GAMMA:
			return V4L2_CID_GAMMA;
		case YARP_FEATURE_GAIN:
			return V4L2_CID_GAIN;
		case YARP_FEATURE_IRIS:
			return V4L2_CID_IRIS_ABSOLUTE;

			//         case YARP_FEATURE_WHITE_BALANCE:  -> this has to e mapped on the
			//         couple V4L2_CID_BLUE_BALANCE && V4L2_CID_RED_BALANCE

			//////////////////////////
			// not yet implemented  //
			//////////////////////////
			//         case YARP_FEATURE_FOCUS:          return DC1394_FEATURE_FOCUS;
			//         case YARP_FEATURE_TEMPERATURE:    return
			//         DC1394_FEATURE_TEMPERATURE; case YARP_FEATURE_TRIGGER: return
			//         DC1394_FEATURE_TRIGGER; case YARP_FEATURE_TRIGGER_DELAY:  return
			//         DC1394_FEATURE_TRIGGER_DELAY; case YARP_FEATURE_FRAME_RATE:
			//         return DC1394_FEATURE_FRAME_RATE; case YARP_FEATURE_ZOOM: return
			//         DC1394_FEATURE_ZOOM; case YARP_FEATURE_PAN:            return
			//         DC1394_FEATURE_PAN; case YARP_FEATURE_TILT:           return
			//         DC1394_FEATURE_TILT;
	}
	return NOT_PRESENT;
}

V4L_camera::V4L_camera() : pythonCameraHelper_(nullptr)
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

	param.camModel = ULTRAPYTON;

	_v4lconvert_data = YARP_NULLPTR;
	timeTot = 0;

	param.user_width = DEFAULT_WIDTH;
	param.user_height = DEFAULT_HEIGHT;

	use_exposure_absolute = false;
}

yarp::os::Stamp V4L_camera::getLastInputStamp()
{
	return timeStamp;
}

int V4L_camera::convertV4L_to_YARP_format(int format)
{
	switch (format)
	{
		case V4L2_PIX_FMT_GREY:
			return VOCAB_PIXEL_MONO;
		case V4L2_PIX_FMT_Y16:
			return VOCAB_PIXEL_MONO16;
		case V4L2_PIX_FMT_RGB24:
			return VOCAB_PIXEL_RGB;
			//     case V4L2_PIX_FMT_ABGR32  : return VOCAB_PIXEL_BGRA; //unsupported by
			//     linux travis configuration
		case V4L2_PIX_FMT_BGR24:
			return VOCAB_PIXEL_BGR;
		case V4L2_PIX_FMT_SGRBG8:
			return VOCAB_PIXEL_ENCODING_BAYER_GRBG8;
		case V4L2_PIX_FMT_SBGGR8:
			return VOCAB_PIXEL_ENCODING_BAYER_BGGR8;
		case V4L2_PIX_FMT_SBGGR16:
			return VOCAB_PIXEL_ENCODING_BAYER_BGGR16;
		case V4L2_PIX_FMT_SGBRG8:
			return VOCAB_PIXEL_ENCODING_BAYER_GBRG8;
		case V4L2_PIX_FMT_SRGGB8:
			return VOCAB_PIXEL_ENCODING_BAYER_RGGB8;
		case V4L2_PIX_FMT_YUV420:
			return VOCAB_PIXEL_YUV_420;
		case V4L2_PIX_FMT_YUV444:
			return VOCAB_PIXEL_YUV_444;
		case V4L2_PIX_FMT_YYUV:
			return VOCAB_PIXEL_YUV_422;
		case V4L2_PIX_FMT_YUV411P:
			return VOCAB_PIXEL_YUV_411;
	}
	return NOT_PRESENT;
}
/**
 *    open device
 */
bool V4L_camera::open(yarp::os::Searchable &config)
{
	struct stat st;
	yCTrace(ULTRAPYTHON) << "input params are " << config.toString();

	if (!fromConfig(config))
	{
		return false;
	}

	if (param.camModel == ULTRAPYTON)
	{
		yCTrace(ULTRAPYTHON) << "ULTRAPYTON";
		if (!pythonCameraHelper_.openAll())
			return false;
		configured = true;
		yarp::os::Time::delay(0.5);
		return true;
	}

	return false;
}

int V4L_camera::getRgbHeight()
{
	return height();
}

int V4L_camera::getRgbWidth()
{
	return width();
}

bool V4L_camera::getRgbSupportedConfigurations(yarp::sig::VectorOf<CameraConfig> &configurations)
{
	yCError(ULTRAPYTHON) << "getRgbSupportedConfigurations - not supported";
	return false;
}
bool V4L_camera::getRgbResolution(int &width, int &height)
{
	width = param.user_width;
	height = param.user_height;
	return true;
}

bool V4L_camera::setRgbResolution(int width, int height)
{
	yCError(ULTRAPYTHON) << "setRgbResolution - not supported";
	return false;
}

bool V4L_camera::getRgbFOV(double &horizontalFov, double &verticalFov)
{
	yCError(ULTRAPYTHON) << "getRgbFOV - not supported";
	return false;
}

bool V4L_camera::setRgbFOV(double horizontalFov, double verticalFov)
{
	yCError(ULTRAPYTHON) << "setRgbFOV - not supported";
	return false;
}

bool V4L_camera::getRgbIntrinsicParam(yarp::os::Property &intrinsic)
{
	yCError(ULTRAPYTHON) << "getRgbIntrinsicParam - not supported";
	return false;
}

bool V4L_camera::getRgbMirroring(bool &mirror)
{
	yCError(ULTRAPYTHON) << "getRgbMirroring - not supported";
	return false;
}

bool V4L_camera::setRgbMirroring(bool mirror)
{
	yCError(ULTRAPYTHON) << "setRgbMirroring - not supported";
	return false;
}

bool V4L_camera::fromConfig(yarp::os::Searchable &config)
{
	if (!config.check("camModel"))
	{
		yCInfo(ULTRAPYTHON) << "No 'camModel' was specified, working with ulrapython";
	}

	auto tmp = config.find("camModel");
	if (tmp.asString() != pythonCameraHelper_.ultraPythonName)
	{
		yCError(ULTRAPYTHON) << "camModel - not supported";
		return false;
	}

	param.camModel = ULTRAPYTON;

	if (config.check("verbose"))
	{
		verbose = true;
	}

	if (param.camModel == ULTRAPYTON)
	{
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
			param.user_height = 1024;
			param.user_width = 2560;
			if (1000.0 / (double)period > UltraPythonCameraHelper::hiresFrameRate_)
			{
				yCWarning(ULTRAPYTHON) << "FPS exceed suggested FPS for hires:" << 1000.0 / (double)period << " suggested:" << UltraPythonCameraHelper::hiresFrameRate_;
			}
		}
		else
		{
			yCDebug(ULTRAPYTHON) << "Python cam sub-sampling ";
			pythonCameraHelper_.setSubsamplingProperty(true);
			param.user_height = 512;
			param.user_width = 1280;
			if (1000.0 / (double)period > UltraPythonCameraHelper::lowresFrameRate_)
			{
				yCWarning(ULTRAPYTHON) << "FPS exceed suggested FPS for lowres:" << 1000.0 / (double)period << " suggested:" << UltraPythonCameraHelper::lowresFrameRate_;
			}
		}
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

	yCDebug(ULTRAPYTHON) << "Ultrapython with the configuration: " << param.user_width << "x" << param.user_height << "; camModel is " << param.camModel;
	return true;
}

/**
 *    close device
 */
bool V4L_camera::close()
{
	yCTrace(ULTRAPYTHON);

	if (param.camModel == ULTRAPYTON)
	{
		return pythonCameraHelper_.closeAll();
	}

	return false;
}

void V4L_camera::pythonPreprocess(const void *pythonbuffer, size_t size)
{
	// Nothing to do
}

// IFrameGrabberRgb Interface
bool V4L_camera::getRgbBuffer(unsigned char *buffer)
{
	if (!configured)
	{
		yCError(ULTRAPYTHON) << "unable to get the buffer, device uninitialized";
		return false;
	}

	mutex.wait();

	if (param.camModel == ULTRAPYTON)
	{
		static Statistics stat("frames read by YARP", pythonCameraHelper_.getCurrentExposure());
		if (pythonCameraHelper_.step(buffer))
		{
			stat.add();
		}
		else
		{
			yCError(ULTRAPYTHON) << "Failed acquiring new frame";
		}
	}

	mutex.post();
	return true;
}

int V4L_camera::height() const
{
	return param.user_height;
}

int V4L_camera::width() const
{
	return param.user_width;
}

bool V4L_camera::set_V4L2_control(uint32_t id, double value, bool verbatim)
{
	if (value < 0)
	{
		return false;
	}

	if (param.camModel == ULTRAPYTON)
	{
		return pythonCameraHelper_.setControl(id, value, false);
	}

	return false;
}

bool V4L_camera::check_V4L2_control(uint32_t id)
{
	if (param.camModel == ULTRAPYTON)
	{
		return pythonCameraHelper_.checkControl(id);
	}

	return false;
}

double V4L_camera::get_V4L2_control(uint32_t id, bool verbatim)
{
	if (param.camModel == ULTRAPYTON)
	{
		return pythonCameraHelper_.getControl(id);
	}
	return false;
}

bool V4L_camera::getCameraDescription(CameraDescriptor *camera)
{
	if (param.camModel == ULTRAPYTON)
	{
		camera->busType = BUS_UNKNOWN;
		camera->deviceDescription = "UltraPython camera";
		return true;
	}
	return false;
}

bool V4L_camera::hasFeature(int feature, bool *_hasFeature)
{
	bool tmpMan(false);
	bool tmpAuto(false);
	bool tmpOnce(false);

	if (param.camModel == ULTRAPYTON)
	{
		if (feature == YARP_FEATURE_WHITE_BALANCE)
		{
			tmpMan = pythonCameraHelper_.hasControl(V4L2_CID_RED_BALANCE) && pythonCameraHelper_.hasControl(V4L2_CID_BLUE_BALANCE);
			tmpOnce = check_V4L2_control(V4L2_CID_DO_WHITE_BALANCE);
			tmpAuto = check_V4L2_control(V4L2_CID_AUTO_WHITE_BALANCE);
			*_hasFeature = tmpMan || tmpOnce || tmpAuto;
			return true;
		}
		if (feature == YARP_FEATURE_EXPOSURE)
		{
			*_hasFeature = true;
			return true;
		}

		*_hasFeature = pythonCameraHelper_.hasControl(convertYARP_to_V4L(feature));
		return true;
	}

	return false;
}

bool V4L_camera::setFeature(int feature, double value)
{
	yCDebug(ULTRAPYTHON) << "setFeature 1";
	bool ret = set_V4L2_control(convertYARP_to_V4L(feature), value);
	return ret;
}

bool V4L_camera::getFeature(int feature, double *value)
{
	yCDebug(ULTRAPYTHON) << "getFeature 1";
	double tmp = 0.0;
	tmp = get_V4L2_control(convertYARP_to_V4L(feature));
	if (tmp == -1)
	{
		return false;
	}

	*value = tmp;
	return true;
}

bool V4L_camera::setFeature(int feature, double value1, double value2)
{
	yCDebug(ULTRAPYTHON) << "setFeature 2";
	if (feature == YARP_FEATURE_WHITE_BALANCE)
	{
		bool ret = true;
		ret &= set_V4L2_control(V4L2_CID_AUTO_WHITE_BALANCE, false);
		ret &= set_V4L2_control(V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, V4L2_WHITE_BALANCE_MANUAL);
		ret &= set_V4L2_control(V4L2_CID_RED_BALANCE, value1);
		ret &= set_V4L2_control(V4L2_CID_BLUE_BALANCE, value2);
		return ret;
	}
	return false;
}

bool V4L_camera::getFeature(int feature, double *value1, double *value2)
{
	if (feature == YARP_FEATURE_WHITE_BALANCE)
	{
		*value1 = get_V4L2_control(V4L2_CID_RED_BALANCE);
		*value2 = get_V4L2_control(V4L2_CID_BLUE_BALANCE);
		return !((*value1 == -1) || (*value2 == -1));
	}
	return false;
}

bool V4L_camera::hasOnOff(int feature, bool *_hasOnOff)
{
	if (param.camModel == ULTRAPYTON)
	{
		*_hasOnOff = false;
		return true;
	}

	return false;
}

bool V4L_camera::setActive(int feature, bool onoff)
{
  yCError(ULTRAPYTHON) << "setActive - not supported";
	return false;
}

bool V4L_camera::getActive(int feature, bool *_isActive)
{
  yCDebug(ULTRAPYTHON) << "yyyz";
	if (param.camModel == ULTRAPYTON)
	{
		if (feature == YARP_FEATURE_WHITE_BALANCE)
		{
			return true;
		}
	}

	switch (feature)
	{
		case YARP_FEATURE_WHITE_BALANCE:
		{
			double tmp = get_V4L2_control(V4L2_CID_AUTO_WHITE_BALANCE);
			if (tmp == 1)
			{
				*_isActive = true;
			}
			else
			{
				*_isActive = false;
			}
			break;
		}

		case YARP_FEATURE_EXPOSURE:
		{
			bool _hasMan(false);
			bool _hasMan2(false);
			hasFeature(V4L2_CID_EXPOSURE, &_hasMan) || hasFeature(V4L2_CID_EXPOSURE_ABSOLUTE,
																  &_hasMan2);  // check manual version (normal and asbolute)
			double _hasAuto = get_V4L2_control(V4L2_CID_EXPOSURE_AUTO, true);  // check auto version

			*_isActive = (_hasAuto == V4L2_EXPOSURE_AUTO) || _hasMan || _hasMan2;
			break;
		}

		default:
			*_isActive = true;
			break;
	}

	return true;
}

bool V4L_camera::hasAuto(int feature, bool *_hasAuto)
{
	if (param.camModel == ULTRAPYTON)
	{
		if (feature == YARP_FEATURE_WHITE_BALANCE)
		{
			*_hasAuto = false;
			return true;
		}
		if (feature == YARP_FEATURE_EXPOSURE)
		{
			*_hasAuto = false;
			return true;
		}

		return pythonCameraHelper_.hasAutoControl(convertYARP_to_V4L(feature));
	}
	return false;
}

bool V4L_camera::hasManual(int feature, bool *_hasManual)
{
	if (param.camModel == ULTRAPYTON)
	{
		if (feature == YARP_FEATURE_WHITE_BALANCE)
		{
			*_hasManual = true;
			return true;
		}

		if (feature == YARP_FEATURE_EXPOSURE)
		{
			*_hasManual = true;
			return true;
		}

		*_hasManual = pythonCameraHelper_.hasControl(convertYARP_to_V4L(feature));
		return true;
	}

	return true;
}

bool V4L_camera::hasOnePush(int feature, bool *_hasOnePush)
{
	// I'm not able to map a 'onePush' request on V4L api
	switch (feature)
	{
		case YARP_FEATURE_WHITE_BALANCE:
			*_hasOnePush = check_V4L2_control(V4L2_CID_DO_WHITE_BALANCE);
			return true;

		default:
			*_hasOnePush = false;
			break;
	}
	return true;
}

bool V4L_camera::setMode(int feature, FeatureMode mode)
{
	bool _tmpAuto;
	bool ret = false;
	switch (feature)
	{
		case YARP_FEATURE_WHITE_BALANCE:
			if (mode == MODE_AUTO)
			{
				ret = set_V4L2_control(V4L2_CID_AUTO_WHITE_BALANCE, true);
			}
			else
			{
				ret = set_V4L2_control(V4L2_CID_AUTO_WHITE_BALANCE, false);
			}
			break;

		case YARP_FEATURE_EXPOSURE:

			hasAuto(V4L2_CID_EXPOSURE_AUTO, &_tmpAuto);

			if (_tmpAuto)
			{
				if (mode == MODE_AUTO)
				{
					ret = set_V4L2_control(V4L2_CID_EXPOSURE_AUTO, true);
				}
				else
				{
					ret = set_V4L2_control(V4L2_CID_EXPOSURE_AUTO, false);
				}
			}
			else
			{
				ret = mode != MODE_AUTO;
			}
			break;

		case YARP_FEATURE_GAIN:
			if (mode == MODE_AUTO)
			{
				yCInfo(ULTRAPYTHON) << "GAIN: set mode auto";
				ret = set_V4L2_control(V4L2_CID_AUTOGAIN, true);
			}
			else
			{
				yCInfo(ULTRAPYTHON) << "GAIN: set mode manual";
				ret = set_V4L2_control(V4L2_CID_AUTOGAIN, false);
			}
			break;

		case YARP_FEATURE_BRIGHTNESS:
		{
			hasAuto(YARP_FEATURE_BRIGHTNESS, &_tmpAuto);

			if (_tmpAuto)
			{
				if (mode == MODE_AUTO)
				{
					ret = set_V4L2_control(V4L2_CID_AUTOBRIGHTNESS, true);
				}
				else
				{
					ret = set_V4L2_control(V4L2_CID_AUTOBRIGHTNESS, false);
				}
			}
			else
			{
				ret = mode != MODE_AUTO;
			}
			break;
		}

		case YARP_FEATURE_HUE:
			if (mode == MODE_AUTO)
			{
				ret = set_V4L2_control(V4L2_CID_HUE_AUTO, true);
			}
			else
			{
				ret = set_V4L2_control(V4L2_CID_HUE_AUTO, false);
			}
			break;

		default:
			yCError(ULTRAPYTHON) << "Feature " << feature << " does not support auto mode";
			break;
	}
	return ret;
}

bool V4L_camera::getMode(int feature, FeatureMode *mode)
{
	bool _tmpAuto;
	switch (feature)
	{
		case YARP_FEATURE_WHITE_BALANCE:
		{
			double ret = get_V4L2_control(V4L2_CID_AUTO_WHITE_BALANCE);
			*mode = toFeatureMode(ret != 0.0);
			break;
		}

		case YARP_FEATURE_EXPOSURE:
		{
			double ret = get_V4L2_control(V4L2_CID_EXPOSURE_AUTO);
			if (ret == -1.0)
			{
				*mode = MODE_MANUAL;
				break;
			}

			if (ret == V4L2_EXPOSURE_MANUAL)
			{
				*mode = MODE_MANUAL;
			}
			else
			{
				*mode = MODE_AUTO;
			}
			break;
		}

		case YARP_FEATURE_BRIGHTNESS:
			hasAuto(YARP_FEATURE_BRIGHTNESS, &_tmpAuto);
			*mode = toFeatureMode(_tmpAuto);
			if (!_tmpAuto)
			{
				*mode = MODE_MANUAL;
			}
			else
			{
				double ret = get_V4L2_control(V4L2_CID_AUTOBRIGHTNESS);
				*mode = toFeatureMode(ret != 0.0);
			}
			break;

		case YARP_FEATURE_GAIN:
			hasAuto(YARP_FEATURE_GAIN, &_tmpAuto);
			*mode = toFeatureMode(_tmpAuto);
			if (!_tmpAuto)
			{
				*mode = MODE_MANUAL;
			}
			else
			{
				double ret = get_V4L2_control(V4L2_CID_AUTOGAIN);
				*mode = toFeatureMode(ret != 0.0);
			}
			break;

		case YARP_FEATURE_HUE:
			hasAuto(YARP_FEATURE_HUE, &_tmpAuto);
			*mode = toFeatureMode(_tmpAuto);
			if (!_tmpAuto)
			{
				*mode = MODE_MANUAL;
			}
			else
			{
				double ret = get_V4L2_control(V4L2_CID_HUE_AUTO);
				*mode = toFeatureMode(ret != 0.0);
			}
			break;

		default:
			*mode = MODE_MANUAL;
			break;
	}
	return true;
}

bool V4L_camera::setOnePush(int feature)
{
	// I'm not able to map a 'onePush' request on each V4L api
	if (feature == YARP_FEATURE_WHITE_BALANCE)
	{
		return set_V4L2_control(V4L2_CID_DO_WHITE_BALANCE, true);
	}
	return false;
}
