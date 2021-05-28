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

#include <yarp/os/LogStream.h>

#include <string>

#include "Statistics.h"

YARP_LOG_COMPONENT(ULTRAPYTHONSTAT, "yarp.device.UltraPythonStatistics")

Statistics::Statistics(const std::string &info, double exposure) : info_(info), exposure_(exposure)
{
	timeStart_ = yarp::os::Time::now();
};

void Statistics::add()
{
	++frameCounter_;
	double timeNow = yarp::os::Time::now();
	double timeElapsed;
	if ((timeElapsed = timeNow - timeStart_) >= statPeriod_)
	{
		latestFps_ = (static_cast<double>(frameCounter_)) / statPeriod_;
		yCInfo(ULTRAPYTHONSTAT) << info_ << " frame number:" << frameCounter_ << " fps:" << latestFps_ << " interval:" << timeElapsed << " sec."
							<< " exposition:" << exposure_ << " msec.";
		frameCounter_ = 0;
		timeStart_ = timeNow;
	}
}

double Statistics::getFps() const
{
	return latestFps_;
}

void Statistics::setExposure(double value)
{
	exposure_ = value;
}
