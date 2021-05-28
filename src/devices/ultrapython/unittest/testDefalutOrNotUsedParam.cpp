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

#include <linux/v4l2-controls.h>
#include <yarp/dev/FrameGrabberInterfaces.h>

#include <chrono>
#include <thread>

#include "../Statistics.h"
#include "../UltraPythonCameraHelper.h"
#include "CApiMock.h"
#include "common.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std::chrono_literals;
using namespace testing;

TEST(UltraPython, notusedparams_ok)
{
	// given
	InterfaceFoCApiMock *interface = new InterfaceFoCApiMock();
	UltraPythonCameraHelper helper(interface);

	// when
	bool force = helper.getForceFormatProperty();
	bool crop = helper.getCropEnabledProperty();
	bool honor = helper.getHonorFps();
	double step = helper.getStepPeriod();

	// then
	EXPECT_FALSE(crop);
	EXPECT_TRUE(force);
	EXPECT_FALSE(honor);
	EXPECT_EQ(40, step);

	delete interface;
}

TEST(UltraPython, hasControls_ok)
{
	// given
	InterfaceFoCApiMock *interface = new InterfaceFoCApiMock();
	UltraPythonCameraHelper helper(interface);

	// when
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_BRIGHTNESS));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_SHUTTER));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_EXPOSURE));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_GAIN));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_RED_GAIN));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_BLUE_GAIN));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_GREEN_GAIN));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_EXPOSURE_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_BRIGHTNESS_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_FPS));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_HONOR_FPS));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_SUBSAMPLING));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_RED_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_BLUE_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::exists(YARP_FEATURE_GREEN_GAIN_ABSOLUTE));

	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_BRIGHTNESS));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_SHUTTER));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_EXPOSURE));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_GAIN));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_RED_GAIN));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_BLUE_GAIN));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_GREEN_GAIN));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_EXPOSURE_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_BRIGHTNESS_ABSOLUTE));
	EXPECT_FALSE(FeatureHelper::existsForWrite(YARP_FEATURE_FPS));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_HONOR_FPS));
	EXPECT_FALSE(FeatureHelper::existsForWrite(YARP_FEATURE_SUBSAMPLING));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_RED_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_BLUE_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForWrite(YARP_FEATURE_GREEN_GAIN_ABSOLUTE));

	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_BRIGHTNESS));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_SHUTTER));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_EXPOSURE));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_GAIN));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_RED_GAIN));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_BLUE_GAIN));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_GREEN_GAIN));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_EXPOSURE_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_BRIGHTNESS_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_FPS));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_HONOR_FPS));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_SUBSAMPLING));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_RED_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_BLUE_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::existsForRead(YARP_FEATURE_GREEN_GAIN_ABSOLUTE));

	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_BRIGHTNESS));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_SHUTTER));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_EXPOSURE));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_GAIN));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_RED_GAIN));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_BLUE_GAIN));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_GREEN_GAIN));
	EXPECT_TRUE(FeatureHelper::isAbsolute(YARP_FEATURE_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::isAbsolute(YARP_FEATURE_EXPOSURE_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::isAbsolute(YARP_FEATURE_BRIGHTNESS_ABSOLUTE));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_FPS));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_HONOR_FPS));
	EXPECT_FALSE(FeatureHelper::isAbsolute(YARP_FEATURE_SUBSAMPLING));
	EXPECT_TRUE(FeatureHelper::isAbsolute(YARP_FEATURE_RED_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::isAbsolute(YARP_FEATURE_BLUE_GAIN_ABSOLUTE));
	EXPECT_TRUE(FeatureHelper::isAbsolute(YARP_FEATURE_GREEN_GAIN_ABSOLUTE));

	delete interface;
}