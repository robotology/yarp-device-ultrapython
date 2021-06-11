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
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/Property.h>

#include <iostream>
#include <string>

#include "../common/common.h"

struct CliArg {
  int code;
  double value;
};

class UltraPythonCli {
 public:
  /**
   * @brief Construct a new Ultra Python Cli object
   * @param grabber Pointer to object representing the camera features
   * controller
   */
  UltraPythonCli(yarp::dev::IFrameGrabberControls* grabber);

  /**
   * @brief Destroy the Ultra Python Cli object
   */
  ~UltraPythonCli();

  /**
   * @brief Used to parse command line arguments
   *
   * @param argc
   * @param argv
   * @param args_map map of arguments: can be --help, --remote, --set, --get.
   * @return true if arguments could be parsed correctly, false otherwise.
   */
  bool ParseArgs(int argc, char* argv[], std::map<std::string, std::string>& args_map_);

  /**
   * @brief Verifies the status of the Yarp network and sets the properties
   *        of the frame grabber
   * @param remotePort string containing the port at which the grabber connects
   * to
   * @param grabber pointer to the frame grabber object to be filled
   * @return true if successful, false otherwise.
   */
  bool InitYarpCommunication(const std::string& remotePort);

  /**
   * @brief Splits a string according to a custom separator
   *
   * @param c string to be split
   * @param separator character delimiter
   * @return The separated substrings in a vector
   */
  std::vector<std::string> splitString(const std::string& c,
                                       const char* separator);

 private:
  /**
   * @brief Device object containing connection properties needed to instantiate
   *        the frame grabber.
   */
  yarp::dev::PolyDriver device_;

  /**
   * @brief Container of connection properties of the device.
   */
  yarp::os::Property property_;

  /**
   * @brief Pointer to camera features grabber
   *
   */
  yarp::dev::IFrameGrabberControls* grabber_;
};
