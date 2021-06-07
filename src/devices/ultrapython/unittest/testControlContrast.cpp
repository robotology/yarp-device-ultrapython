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

#include <chrono>
#include <thread>

#include "../UltraPythonCameraHelper.h"
#include "CApiMock.h"
#include "../Statistics.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std::chrono_literals;
using namespace testing;

TEST(UltraPython, setContrast_absolute_ok) {
    // given 
    InterfaceFoCApiMock *interface = new InterfaceFoCApiMock();
    UltraPythonCameraHelper helper(interface);
    helper.setStepPeriod(100);

    struct v4l2_control control1;
    control1.id = UltraPythonCameraHelper::V4L2_CONTRAST_ULTRA_PYTHON;
    control1.value = 20;
    EXPECT_CALL(*interface, ioctl_query_c(_, _, _))
        .WillOnce(Return(1));
    EXPECT_CALL(*interface, ioctl_control_c(_, VIDIOC_S_CTRL, control1)).Times(1);

    // when
    bool result = helper.setControl(UltraPythonCameraHelper::V4L2_CONTRAST_ULTRA_PYTHON, 20, true);

    // then
    EXPECT_TRUE(result);

    delete interface;
}

TEST(UltraPython, setContrast_relative_ok) {
  // given
  InterfaceFoCApiMock *interface = new InterfaceFoCApiMock();
  	UltraPythonCameraHelper helper(interface);
  helper.setStepPeriod(100);

  struct v4l2_queryctrl queryctrl;
  queryctrl.maximum = 0;
  queryctrl.minimum = 100;
  queryctrl.flags = 0;
  struct v4l2_control control1;
  control1.id = UltraPythonCameraHelper::V4L2_CONTRAST_ULTRA_PYTHON;
  control1.value = 50;
  EXPECT_CALL(*interface, ioctl_query_c(_, VIDIOC_QUERYCTRL, _))
      .WillOnce(DoAll(SetArgReferee<2>(queryctrl), Return(1)));
  EXPECT_CALL(*interface, ioctl_control_c(_, VIDIOC_S_CTRL, control1)).Times(1);

  // when
  bool res = helper.setControl(UltraPythonCameraHelper::V4L2_CONTRAST_ULTRA_PYTHON, 0.50, false);

  // then
  EXPECT_TRUE(res);

  delete interface;
}

TEST(UltraPython, getContrast_relative_ok) {

    // given
    InterfaceFoCApiMock *interface = new InterfaceFoCApiMock();
    UltraPythonCameraHelper helper(interface);
    helper.setStepPeriod(100);

    struct v4l2_queryctrl queryctrl;
    queryctrl.maximum = 0;
    queryctrl.minimum = 100;
    queryctrl.flags = 0;
    struct v4l2_control control1;
    control1.id = UltraPythonCameraHelper::V4L2_CONTRAST_ULTRA_PYTHON;
    control1.value = 50;

    EXPECT_CALL(*interface, ioctl_query_c(_, VIDIOC_QUERYCTRL, _)).
        WillOnce(DoAll(SetArgReferee<2>(queryctrl), Return(1)));
    EXPECT_CALL(*interface, ioctl_control_c(_, VIDIOC_G_CTRL, _)).
        WillOnce(DoAll(SetArgReferee<2>(control1), Return(1)));

    // when
    double result = helper.getControl(UltraPythonCameraHelper::V4L2_CONTRAST_ULTRA_PYTHON, false);

    // then
    EXPECT_EQ(result, 0.5);

    delete interface;        
}


TEST(UltraPython, getContrast_absolute_ok) {

    // given
    InterfaceFoCApiMock *interface = new InterfaceFoCApiMock();
    UltraPythonCameraHelper helper(interface);
    helper.setStepPeriod(100);

    struct v4l2_queryctrl queryctrl;
    queryctrl.maximum = 0;
    queryctrl.minimum = 100;
    queryctrl.flags = 0;
    struct v4l2_control control1;
    control1.id = UltraPythonCameraHelper::V4L2_CONTRAST_ULTRA_PYTHON;
    control1.value = 50;

    EXPECT_CALL(*interface, ioctl_query_c(_, VIDIOC_QUERYCTRL, _)).
        WillOnce(DoAll(SetArgReferee<2>(queryctrl), Return(1)));
    EXPECT_CALL(*interface, ioctl_control_c(_, VIDIOC_G_CTRL, _)).
        WillOnce(DoAll(SetArgReferee<2>(control1), Return(1)));

    // when
    double result = helper.getControl(UltraPythonCameraHelper::V4L2_CONTRAST_ULTRA_PYTHON, true);

    // then
    EXPECT_EQ(result, 50);

    delete interface;        
}
