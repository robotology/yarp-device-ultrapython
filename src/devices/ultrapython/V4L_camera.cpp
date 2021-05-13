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

using namespace yarp::os;
using namespace yarp::dev;

V4L_camera::V4L_camera() : pythonCameraHelper_(nullptr)
{
//DONE
}

bool V4L_camera::open(yarp::os::Searchable &config)
{
}

int V4L_camera::getRgbHeight()
{

}

int V4L_camera::getRgbWidth()
{
}

bool V4L_camera::getRgbResolution(int &width, int &height)
{

}

bool V4L_camera::fromConfig(yarp::os::Searchable &config)
{

}

bool V4L_camera::close()
{

}

void V4L_camera::pythonPreprocess(const void *pythonbuffer, size_t size)
{
	// Nothing to do
}

// IFrameGrabberRgb Interface
bool V4L_camera::getRgbBuffer(unsigned char *buffer)
{

}

int V4L_camera::height() const
{
	
}

int V4L_camera::width() const
{
	
}

bool V4L_camera::getCameraDescription(CameraDescriptor *camera)
{

}

bool V4L_camera::hasFeature(int feature, bool *_hasFeature)
{
	
}

bool V4L_camera::getFeature(int feature, double *value)
{

}

bool V4L_camera::getFeature(int feature, double *value1, double *value2)
{

}

bool V4L_camera::setFeature(int feature, double value)
{

}

bool V4L_camera::setFeature(int feature, double value1, double value2)
{
}

bool V4L_camera::hasOnOff(int feature, bool *_hasOnOff)
{
	*_hasOnOff = false;
	return true;
}

bool V4L_camera::getActive(int feature, bool *_isActive)
{
	*_isActive = true;
	return true;
}

bool V4L_camera::hasAuto(int feature, bool *_hasAuto)
{
	*_hasAuto = pythonCameraHelper_.hasAutoControl(pythonCameraHelper_.remapControlYARPtoV4L(feature));
	return true;
}

bool V4L_camera::hasManual(int feature, bool *_hasManual)
{
	*_hasManual = pythonCameraHelper_.hasManualControl(pythonCameraHelper_.remapControlYARPtoV4L(feature));
	return true;
}

bool V4L_camera::setMode(int feature, FeatureMode mode)
{
	yCError(ULTRAPYTHON) << "Feature " << feature << " does not support auto mode";
	return false;
}

bool V4L_camera::getMode(int feature, FeatureMode *mode)
{
	*mode = MODE_MANUAL;
	return true;
}

bool V4L_camera::setActive(int feature, bool onoff)
{
	yCError(ULTRAPYTHON) << "setActive - not supported";
	return false;
}

bool V4L_camera::hasOnePush(int feature, bool *_hasOnePush)
{
	yCError(ULTRAPYTHON) << "hasOnePush - not supported";
	*_hasOnePush = false;
	return false;
}

bool V4L_camera::setOnePush(int feature)
{
	yCError(ULTRAPYTHON) << "setOnePush - not supported";
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

bool V4L_camera::getRgbSupportedConfigurations(yarp::sig::VectorOf<CameraConfig> &configurations)
{
	yCError(ULTRAPYTHON) << "getRgbSupportedConfigurations - not supported";
	return false;
}

bool V4L_camera::setRgbResolution(int width, int height)
{
	yCError(ULTRAPYTHON) << "setRgbResolution - not supported";
	return false;
}