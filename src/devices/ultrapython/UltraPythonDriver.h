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

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/FrameGrabberInterfaces.h>
#include <yarp/dev/IPreciselyTimed.h>
#include <yarp/dev/IVisualParams.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>

#include "UltraPythonCameraHelper.h"

class UltraPythonDriver : 	public yarp::dev::DeviceDriver, 
							public yarp::dev::IPreciselyTimed, 
							public yarp::dev::IFrameGrabberControls, 
							public yarp::dev::IFrameGrabberImage
							//public yarp::dev::IRgbVisualParams 
{
	UltraPythonDriver(const UltraPythonDriver &) = delete;
	void operator=(const UltraPythonDriver &) = delete;

   public:
	UltraPythonDriver();
	~UltraPythonDriver() override;

	// Implements DeviceDriver
	bool open(yarp::os::Searchable &config) override;
	bool close() override;

	// Implements IFrameGrabberImage basic interface.
	int height() const override;
	int width() const override;
	bool getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb> &image) override;

	// Implements the IPreciselyTimed interface.
	yarp::os::Stamp getLastInputStamp() override;

	// Implementation of IFrameGrabberControls interface
	bool getCameraDescription(CameraDescriptor *camera) override;
	bool hasFeature(int feature, bool *hasFeature) override;
	bool setFeature(int feature, double value) override;
	bool getFeature(int feature, double *value) override;
	bool setFeature(int feature, double value1, double value2) override;
	bool getFeature(int feature, double *value1, double *value2) override;
	bool hasOnOff(int feature, bool *HasOnOff) override;
	bool setActive(int feature, bool onoff) override;
	bool getActive(int feature, bool *isActive) override;
	bool hasAuto(int feature, bool *hasAuto) override;
	bool hasManual(int feature, bool *hasManual) override;
	bool hasOnePush(int feature, bool *hasOnePush) override;
	bool setMode(int feature, FeatureMode mode) override;
	bool getMode(int feature, FeatureMode *mode) override;
	bool setOnePush(int feature) override;

   private:
	yarp::os::Semaphore mutex;
	bool configured_{false};

	bool fromConfig(yarp::os::Searchable &config);

	UltraPythonCameraHelper pythonCameraHelper_;
	void pythonPreprocess(const void *pythonbuffer, size_t size);

	int remapControlYARPtoXilinx(int feature) const;
};
