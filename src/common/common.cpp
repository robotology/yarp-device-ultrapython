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

#include "common.h"

#include <yarp/dev/FrameGrabberInterfaces.h>

bool FeatureHelper::existsForWrite(int feature)
{
	switch (feature)
	{
		case YARP_FEATURE_BRIGHTNESS:
		case YARP_FEATURE_CONTRAST:
		case YARP_FEATURE_SHUTTER:
		case YARP_FEATURE_EXPOSURE:
		case YARP_FEATURE_GAIN:
		case YARP_FEATURE_RED_GAIN:
		case YARP_FEATURE_BLUE_GAIN:
		case YARP_FEATURE_GREEN_GAIN:
		case YARP_FEATURE_GAIN_ABSOLUTE:
		case YARP_FEATURE_EXPOSURE_ABSOLUTE:
		case YARP_FEATURE_BRIGHTNESS_ABSOLUTE:
		case YARP_FEATURE_CONTRAST_ABSOLUTE:
		case YARP_FEATURE_RED_GAIN_ABSOLUTE:
		case YARP_FEATURE_BLUE_GAIN_ABSOLUTE:
		case YARP_FEATURE_GREEN_GAIN_ABSOLUTE:
		case YARP_FEATURE_HONOR_FPS:
			return true;
	}
	return false;
}
bool FeatureHelper::existsForRead(int feature)
{
	switch (feature)
	{
		case YARP_FEATURE_BRIGHTNESS:
		case YARP_FEATURE_CONTRAST:
		case YARP_FEATURE_SHUTTER:
		case YARP_FEATURE_EXPOSURE:
		case YARP_FEATURE_GAIN:
		case YARP_FEATURE_RED_GAIN:
		case YARP_FEATURE_BLUE_GAIN:
		case YARP_FEATURE_GREEN_GAIN:
		case YARP_FEATURE_GAIN_ABSOLUTE:
		case YARP_FEATURE_EXPOSURE_ABSOLUTE:
		case YARP_FEATURE_BRIGHTNESS_ABSOLUTE:
		case YARP_FEATURE_CONTRAST_ABSOLUTE:
		case YARP_FEATURE_FPS:
		case YARP_FEATURE_SUBSAMPLING:
		case YARP_FEATURE_RED_GAIN_ABSOLUTE:
		case YARP_FEATURE_BLUE_GAIN_ABSOLUTE:
		case YARP_FEATURE_GREEN_GAIN_ABSOLUTE:
		case YARP_FEATURE_HONOR_FPS:
			return true;
	}
	return false;
}

bool FeatureHelper::exists(int feature)
{
	return existsForWrite(feature) || existsForRead(feature);
}

bool FeatureHelper::isAbsolute(int feature)
{
	switch (feature)
	{
		case YARP_FEATURE_GAIN_ABSOLUTE:
		case YARP_FEATURE_EXPOSURE_ABSOLUTE:
		case YARP_FEATURE_BRIGHTNESS_ABSOLUTE:
		case YARP_FEATURE_CONTRAST_ABSOLUTE:
		case YARP_FEATURE_RED_GAIN_ABSOLUTE:
		case YARP_FEATURE_BLUE_GAIN_ABSOLUTE:
		case YARP_FEATURE_GREEN_GAIN_ABSOLUTE:
			return true;
	}
	return false;
}

bool FeatureHelper::isV4Lcontrol(int feature)
{
	switch (feature)
	{
		case YARP_FEATURE_HONOR_FPS:
		case YARP_FEATURE_FPS:
		case YARP_FEATURE_SUBSAMPLING:
			return false;
	}
	return true;
}