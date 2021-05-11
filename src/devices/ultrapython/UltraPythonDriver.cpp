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

#include "UltraPythonCameraLogComponent.h"
#include "V4L_camera.h"

using namespace yarp::os;
using namespace yarp::dev;

///////////////// generic device //////////////////////////

UltraPythonDriver::UltraPythonDriver()
{
	yCTrace(ULTRAPYTHON) << "Create drive";
	pixelType = VOCAB_PIXEL_RGB;
}

UltraPythonDriver::~UltraPythonDriver()
{
	yCTrace(ULTRAPYTHON) << "Destroy drive";
}

bool UltraPythonDriver::open(yarp::os::Searchable &config)
{
	// open OS dependant device
	yCTrace(ULTRAPYTHON) << "input params are " << config.toString();

#if defined(_MSC_VER)
	os_device = (DeviceDriver *)new WIN_camera;
#elif defined __unix
	os_device = (DeviceDriver *)new V4L_camera;
#endif

	yarp::os::Property prop;
	prop.fromString(config.toString());
	if (!prop.check("pixelType"))
	{
		switch (pixelType)
		{
			case VOCAB_PIXEL_MONO:
				prop.put("pixelType", VOCAB_PIXEL_MONO);
				break;

			case VOCAB_PIXEL_RGB:
			default:
				prop.put("pixelType", VOCAB_PIXEL_RGB);
				break;
		}
	}
	if (!os_device->open(prop))
	{
		delete os_device;
		return false;
	}

	os_device->view(deviceRgb);
	os_device->view(deviceControls);
	os_device->view(deviceTimed);
	os_device->view(deviceRgbVisualParam);

	if (deviceRgb != nullptr)
	{
		_width = deviceRgb->width();
		_height = deviceRgb->height();
	}
	return true;
}

bool UltraPythonDriver::close()
{
	// close OS dependant device
	os_device->close();
	delete os_device;
	return true;
}

int UltraPythonDriver::width() const
{
	if (deviceRgb != nullptr)
	{
		return deviceRgb->width();
	}
	else
	{
		return 0;
	}
}

int UltraPythonDriver::height() const
{
	if (deviceRgb != nullptr)
	{
		return deviceRgb->height();
	}
	else
	{
		return 0;
	}
}

bool UltraPythonDriver::getRawBuffer(unsigned char *buff)
{
	return false;
}

int UltraPythonDriver::getRawBufferSize()
{
	return 0;
}

bool UltraPythonDriver::getRgbBuffer(unsigned char *buff)
{
	return false;
}

yarp::os::Stamp UltraPythonDriver::getLastInputStamp()
{
	if (deviceTimed != nullptr)
	{
		return deviceTimed->getLastInputStamp();
	}

	return yarp::os::Stamp();
}

int UltraPythonDriver::getRgbHeight()
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->getRgbHeight();
	}
	return 0;
}

int UltraPythonDriver::getRgbWidth()
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->getRgbWidth();
	}
	return 0;
}

bool UltraPythonDriver::getRgbSupportedConfigurations(yarp::sig::VectorOf<CameraConfig> &configurations)
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->getRgbSupportedConfigurations(configurations);
	}
	return false;
}

bool UltraPythonDriver::getRgbResolution(int &width, int &height)
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->getRgbResolution(width, height);
	}
	return false;
}

bool UltraPythonDriver::setRgbResolution(int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		yCError(ULTRAPYTHON) << "usbCamera: invalid width or height";
		return false;
	}
	if (deviceRgbVisualParam != nullptr)
	{
		_width = width;
		_height = height;
		return deviceRgbVisualParam->setRgbResolution(width, height);
	}
	return false;
}

bool UltraPythonDriver::getRgbFOV(double &horizontalFov, double &verticalFov)
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->getRgbFOV(horizontalFov, verticalFov);
	}
	return false;
}

bool UltraPythonDriver::setRgbFOV(double horizontalFov, double verticalFov)
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->setRgbFOV(horizontalFov, verticalFov);
	}
	return false;
}

bool UltraPythonDriver::getRgbIntrinsicParam(yarp::os::Property &intrinsic)
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->getRgbIntrinsicParam(intrinsic);
	}
	return false;
}

bool UltraPythonDriver::getRgbMirroring(bool &mirror)
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->getRgbMirroring(mirror);
	}
	return false;
}

bool UltraPythonDriver::setRgbMirroring(bool mirror)
{
	if (deviceRgbVisualParam != nullptr)
	{
		return deviceRgbVisualParam->setRgbMirroring(mirror);
	}
	return false;
}

bool UltraPythonDriver::getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb> &image)
{
	if ((image.width() != _width) || (image.height() != _height))
	{
		image.resize(_width, _height);
	}
	deviceRgb->getRgbBuffer(image.getRawImage());
	return true;
}

/*  Implementation of IFrameGrabberControls2 interface
 *
 * Actual function will be implemented by OS specific devices
 */

bool UltraPythonDriver::getCameraDescription(CameraDescriptor *camera)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->getCameraDescription(camera);
	}
	return false;
}

bool UltraPythonDriver::hasFeature(int feature, bool *_hasFeature)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->hasFeature(feature, _hasFeature);
	}
	return false;
}

bool UltraPythonDriver::setFeature(int feature, double value)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->setFeature(feature, value);
	}
	return false;
}

bool UltraPythonDriver::getFeature(int feature, double *value)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->getFeature(feature, value);
	}
	return false;
}

bool UltraPythonDriver::getFeature(int feature, double *value1, double *value2)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->getFeature(feature, value1, value2);
	}
	return false;
}

bool UltraPythonDriver::setFeature(int feature, double value1, double value2)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->setFeature(feature, value1, value2);
	}
	return false;
}

bool UltraPythonDriver::hasOnOff(int feature, bool *_hasOnOff)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->hasOnOff(feature, _hasOnOff);
	}
	return false;
}

bool UltraPythonDriver::setActive(int feature, bool onoff)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->setActive(feature, onoff);
	}
	return false;
}

bool UltraPythonDriver::getActive(int feature, bool *isActive)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->getActive(feature, isActive);
	}
	return false;
}

bool UltraPythonDriver::hasAuto(int feature, bool *_hasAuto)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->hasAuto(feature, _hasAuto);
	}
	return false;
}

bool UltraPythonDriver::hasManual(int feature, bool *_hasManual)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->hasManual(feature, _hasManual);
	}
	return false;
}

bool UltraPythonDriver::hasOnePush(int feature, bool *_hasOnePush)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->hasOnePush(feature, _hasOnePush);
	}
	return false;
}

bool UltraPythonDriver::setMode(int feature, FeatureMode mode)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->setMode(feature, mode);
	}
	return false;
}

bool UltraPythonDriver::getMode(int feature, FeatureMode *mode)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->getMode(feature, mode);
	}
	return false;
}

bool UltraPythonDriver::setOnePush(int feature)
{
	if (deviceControls != nullptr)
	{
		return deviceControls->setOnePush(feature);
	}
	return false;
}
