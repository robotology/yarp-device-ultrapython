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

#include "UltraPythonDriverGeneric.h"
#include "UltraPythonCameraLogComponent.h"

#include "V4L_camera.h"

#include <yarp/os/LogStream.h>
#include <yarp/os/Semaphore.h>
#include <yarp/os/Stamp.h>

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/FrameGrabberInterfaces.h>

using namespace yarp::os;
using namespace yarp::dev;

///////////////// generic device //////////////////////////

UltraPythonDriverGeneric::UltraPythonDriverGeneric() {
  // initialize stuff
  yCTrace(ULTRAPYTHON);
}

UltraPythonDriverGeneric::~UltraPythonDriverGeneric() {
  // delete subdevice, of any
  yCTrace(ULTRAPYTHON);
}

bool UltraPythonDriverGeneric::open(yarp::os::Searchable &config) {
  // open OS dependant device
  yCTrace(ULTRAPYTHON) << "input params are " << config.toString();

#if defined(_MSC_VER)
  os_device = (DeviceDriver *)new WIN_camera;
#elif defined __unix
  os_device = (DeviceDriver *)new V4L_camera;
#endif

  yarp::os::Property prop;
  prop.fromString(config.toString());
  if (!prop.check("pixelType")) {
    switch (pixelType) {
    case VOCAB_PIXEL_MONO:
      prop.put("pixelType", VOCAB_PIXEL_MONO);
      break;

    case VOCAB_PIXEL_RGB:
    default:
      prop.put("pixelType", VOCAB_PIXEL_RGB);
      break;
    }
  }
  if (!os_device->open(prop)) {
    delete os_device;
    return false;
  }

  os_device->view(deviceRgb);
  os_device->view(deviceRaw);
  os_device->view(deviceControls);
  os_device->view(deviceTimed);
  os_device->view(deviceRgbVisualParam);

  if (deviceRaw != nullptr) {
    _width = deviceRaw->width();
    _height = deviceRaw->height();
  }

  if (deviceRgb != nullptr) {
    _width = deviceRgb->width();
    _height = deviceRgb->height();
  }
  return true;
}

bool UltraPythonDriverGeneric::close() {
  // close OS dependant device
  os_device->close();
  delete os_device;
  return true;
}

int UltraPythonDriverGeneric::width() const {
  if (deviceRaw != nullptr) {
    return deviceRaw->width();
  }
  if (deviceRgb != nullptr) {
    return deviceRgb->width();
  } else {
    return 0;
  }
}

int UltraPythonDriverGeneric::height() const {
  if (deviceRaw != nullptr) {
    return deviceRaw->height();
  }
  if (deviceRgb != nullptr) {
    return deviceRgb->height();
  } else {
    return 0;
  }
}

bool UltraPythonDriverGeneric::getRawBuffer(unsigned char *buff) { return false; }

int UltraPythonDriverGeneric::getRawBufferSize() { return 0; }

bool UltraPythonDriverGeneric::getRgbBuffer(unsigned char *buff) { return false; }

yarp::os::Stamp UltraPythonDriverGeneric::getLastInputStamp() {
  if (deviceTimed != nullptr) {
    return deviceTimed->getLastInputStamp();
  }

  return yarp::os::Stamp();
}

int UltraPythonDriverGeneric::getRgbHeight() {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->getRgbHeight();
  }
  return 0;
}

int UltraPythonDriverGeneric::getRgbWidth() {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->getRgbWidth();
  }
  return 0;
}

bool UltraPythonDriverGeneric::getRgbSupportedConfigurations(
    yarp::sig::VectorOf<CameraConfig> &configurations) {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->getRgbSupportedConfigurations(configurations);
  }
  return false;
}

bool UltraPythonDriverGeneric::getRgbResolution(int &width, int &height) {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->getRgbResolution(width, height);
  }
  return false;
}

bool UltraPythonDriverGeneric::setRgbResolution(int width, int height) {
  if (width <= 0 || height <= 0) {
    yCError(ULTRAPYTHON) << "usbCamera: invalid width or height";
    return false;
  }
  if (deviceRgbVisualParam != nullptr) {
    _width = width;
    _height = height;
    return deviceRgbVisualParam->setRgbResolution(width, height);
  }
  return false;
}

bool UltraPythonDriverGeneric::getRgbFOV(double &horizontalFov, double &verticalFov) {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->getRgbFOV(horizontalFov, verticalFov);
  }
  return false;
}

bool UltraPythonDriverGeneric::setRgbFOV(double horizontalFov, double verticalFov) {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->setRgbFOV(horizontalFov, verticalFov);
  }
  return false;
}

bool UltraPythonDriverGeneric::getRgbIntrinsicParam(yarp::os::Property &intrinsic) {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->getRgbIntrinsicParam(intrinsic);
  }
  return false;
}

bool UltraPythonDriverGeneric::getRgbMirroring(bool &mirror) {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->getRgbMirroring(mirror);
  }
  return false;
}

bool UltraPythonDriverGeneric::setRgbMirroring(bool mirror) {
  if (deviceRgbVisualParam != nullptr) {
    return deviceRgbVisualParam->setRgbMirroring(mirror);
  }
  return false;
}

//// RGB ///
UltraPythonDriver::UltraPythonDriver() : UltraPythonDriverGeneric() {
  yCTrace(ULTRAPYTHON);
  pixelType = VOCAB_PIXEL_RGB;
}

UltraPythonDriver::~UltraPythonDriver() { yCTrace(ULTRAPYTHON); }

bool UltraPythonDriver::getImage(
    yarp::sig::ImageOf<yarp::sig::PixelRgb> &image) {
  if ((image.width() != _width) || (image.height() != _height)) {
    image.resize(_width, _height);
  }
  deviceRgb->getRgbBuffer(image.getRawImage());
  return true;
}

bool UltraPythonDriver::getImage(
    yarp::sig::ImageOf<yarp::sig::PixelMono> &image) {
  if ((image.width() != _width) || (image.height() != _height)) {
    image.resize(_width, _height);
  }

  deviceRaw->getRawBuffer(image.getRawImage());
  return true;
}

int UltraPythonDriver::width() const { return UltraPythonDriverGeneric::width(); }

int UltraPythonDriver::height() const { return UltraPythonDriverGeneric::height(); }

//// RAW ///
UltraPythonDriverRaw::UltraPythonDriverRaw() : UltraPythonDriverGeneric() {
  yCTrace(ULTRAPYTHON);
  pixelType = VOCAB_PIXEL_MONO;
}

UltraPythonDriverRaw::~UltraPythonDriverRaw() { yCTrace(ULTRAPYTHON); }

bool UltraPythonDriverRaw::getImage(
    yarp::sig::ImageOf<yarp::sig::PixelMono> &image) {
  if ((image.width() != _width) || (image.height() != _height)) {
    image.resize(_width, _height);
  }

  deviceRaw->getRawBuffer(image.getRawImage());
  return true;
}

int UltraPythonDriverRaw::width() const { return UltraPythonDriverGeneric::width(); }

int UltraPythonDriverRaw::height() const { return UltraPythonDriverGeneric::height(); }

/*  Implementation of IFrameGrabberControls2 interface
 *
 * Actual function will be implemented by OS specific devices
 */

bool UltraPythonDriverGeneric::getCameraDescription(CameraDescriptor *camera) {
  if (deviceControls != nullptr) {
    return deviceControls->getCameraDescription(camera);
  }
  return false;
}

bool UltraPythonDriverGeneric::hasFeature(int feature, bool *_hasFeature) {
  if (deviceControls != nullptr) {
    return deviceControls->hasFeature(feature, _hasFeature);
  }
  return false;
}

bool UltraPythonDriverGeneric::setFeature(int feature, double value) {
  if (deviceControls != nullptr) {
    return deviceControls->setFeature(feature, value);
  }
  return false;
}

bool UltraPythonDriverGeneric::getFeature(int feature, double *value) {
  if (deviceControls != nullptr) {
    return deviceControls->getFeature(feature, value);
  }
  return false;
}

bool UltraPythonDriverGeneric::getFeature(int feature, double *value1, double *value2) {
  if (deviceControls != nullptr) {
    return deviceControls->getFeature(feature, value1, value2);
  }
  return false;
}

bool UltraPythonDriverGeneric::setFeature(int feature, double value1, double value2) {
  if (deviceControls != nullptr) {
    return deviceControls->setFeature(feature, value1, value2);
  }
  return false;
}

bool UltraPythonDriverGeneric::hasOnOff(int feature, bool *_hasOnOff) {
  if (deviceControls != nullptr) {
    return deviceControls->hasOnOff(feature, _hasOnOff);
  }
  return false;
}

bool UltraPythonDriverGeneric::setActive(int feature, bool onoff) {
  if (deviceControls != nullptr) {
    return deviceControls->setActive(feature, onoff);
  }
  return false;
}

bool UltraPythonDriverGeneric::getActive(int feature, bool *isActive) {
  if (deviceControls != nullptr) {
    return deviceControls->getActive(feature, isActive);
  }
  return false;
}

bool UltraPythonDriverGeneric::hasAuto(int feature, bool *_hasAuto) {
  if (deviceControls != nullptr) {
    return deviceControls->hasAuto(feature, _hasAuto);
  }
  return false;
}

bool UltraPythonDriverGeneric::hasManual(int feature, bool *_hasManual) {
  if (deviceControls != nullptr) {
    return deviceControls->hasManual(feature, _hasManual);
  }
  return false;
}

bool UltraPythonDriverGeneric::hasOnePush(int feature, bool *_hasOnePush) {
  if (deviceControls != nullptr) {
    return deviceControls->hasOnePush(feature, _hasOnePush);
  }
  return false;
}

bool UltraPythonDriverGeneric::setMode(int feature, FeatureMode mode) {
  if (deviceControls != nullptr) {
    return deviceControls->setMode(feature, mode);
  }
  return false;
}

bool UltraPythonDriverGeneric::getMode(int feature, FeatureMode *mode) {
  if (deviceControls != nullptr) {
    return deviceControls->getMode(feature, mode);
  }
  return false;
}

bool UltraPythonDriverGeneric::setOnePush(int feature) {
  if (deviceControls != nullptr) {
    return deviceControls->setOnePush(feature);
  }
  return false;
}
