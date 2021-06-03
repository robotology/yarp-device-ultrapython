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

/*
Already present in YARP
constexpr int YARP_FEATURE_BRIGHTNESS=;
constexpr int YARP_FEATURE_SHUTTER=;
constexpr int YARP_FEATURE_EXPOSURE=;
constexpr int YARP_FEATURE_GAIN=;
*/

// Additional YARP FEATURE for Ultrapyhton
constexpr int YARP_FEATURE_RED_GAIN = 50;
constexpr int YARP_FEATURE_BLUE_GAIN = 51;
constexpr int YARP_FEATURE_GREEN_GAIN = 52;
constexpr int YARP_FEATURE_GAIN_ABSOLUTE = 60;
constexpr int YARP_FEATURE_EXPOSURE_ABSOLUTE = 61;
constexpr int YARP_FEATURE_BRIGHTNESS_ABSOLUTE = 62;
constexpr int YARP_FEATURE_RED_GAIN_ABSOLUTE = 63;
constexpr int YARP_FEATURE_BLUE_GAIN_ABSOLUTE = 64;
constexpr int YARP_FEATURE_GREEN_GAIN_ABSOLUTE = 65;
constexpr int YARP_FEATURE_FPS = 70;
constexpr int YARP_FEATURE_SUBSAMPLING = 72;
constexpr int YARP_FEATURE_HONOR_FPS = 73;
constexpr int YARP_FEATURE_CONTRAST = 74;
constexpr int YARP_FEATURE_CONTRAST_ABSOLUTE = 75;

class FeatureHelper
{
   public:
	static bool existsForWrite(int feature);
	static bool existsForRead(int feature);
	static bool exists(int feature);
	static bool isAbsolute(int feature);
	static bool isV4Lcontrol(int feature);
};