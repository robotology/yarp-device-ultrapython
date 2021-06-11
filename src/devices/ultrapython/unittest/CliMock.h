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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "yarp/dev/FrameGrabberInterfaces.h"

/**
 * @brief Mocking class created to replace the grabber object
 */
class IFrameGrabberControlsMock : public yarp::dev::IFrameGrabberControls {
 public:
  MOCK_METHOD(bool, setFeature, (int, double), (override));
  MOCK_METHOD(bool, getFeature, (int, double *), (override));

  bool getCameraDescription(CameraDescriptor *camera) override { return true; };
  bool hasFeature(int feature, bool *hasFeature) override { return true; };
  bool setFeature(int feature, double value1, double value2) override {
    return true;
  };
  bool getFeature(int feature, double *value1, double *value2) override {
    return true;
  };
  bool hasOnOff(int feature, bool *HasOnOff) override { return true; };
  bool setActive(int feature, bool onoff) override { return true; };
  bool getActive(int feature, bool *isActive) override { return true; };
  bool hasAuto(int feature, bool *hasAuto) override { return true; };
  bool hasManual(int feature, bool *hasManual) override { return true; };
  bool hasOnePush(int feature, bool *hasOnePush) override { return true; };
  bool setMode(int feature, FeatureMode mode) override { return true; };
  bool getMode(int feature, FeatureMode *mode) override { return true; };
  bool setOnePush(int feature) override { return true; };
};