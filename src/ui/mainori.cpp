
/*
 * Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/FrameGrabberInterfaces.h>
#include <yarp/dev/IPreciselyTimed.h>
#include <yarp/dev/IVisualParams.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Network.h>
#include <yarp/os/Property.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Image.h>

#include <iostream>
#include <ratio>
#include <thread>
#include <chrono>

int main(int argc, char* argv[])
{
	YARP_UNUSED(argc);
	YARP_UNUSED(argv);
	using ImageType = yarp::sig::ImageOf<yarp::sig::PixelRgb>;

	yarp::os::Network yarp;

	 yarp::os::BufferedPort<ImageType> out;
	 out.open("/grabber1");

	yarp::dev::PolyDriver dd;
	yarp::os::Property p;
	p.put("device", "remote_grabber");
	p.put("local", "/pippo");
	p.put("remote", "/grabber/rpc");

	if (!dd.open(p))
	{
		std::cout << "Unable to open PolyDrive for wrapper name:" << std::endl;
		return 0;
	}

	yarp::dev::IFrameGrabberControls* grabber = nullptr;
	dd.view(grabber);

    if(!grabber)
	{
		std::cout << "Unable to view PolyDrive for wrapper name:" << std::endl;
		return 0;
	}
	
	using namespace std::chrono_literals;
	while(1)
	{
		cameraFeature_id_t feature=(cameraFeature_id_t)YARP_FEATURE_GAIN;
		bool value=false;
		bool res=grabber->setFeature(feature,0.8);
		std::cout << res <<"::" <<value<<std::endl;

		std::this_thread::sleep_for(1000ms);
	}
	return 0;
}
