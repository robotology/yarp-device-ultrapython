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

#include "UltraPythonCli.h"

/**
 * @brief Executes the main loop, and parses command line arguments
 * @returns -1 on error, 0 if successful
 */
int main(int argc, char* argv[]) {
  int control_code = 0;
  double value = 0.0;
  std::string port_name = "/grabber";
  std::map<std::string, std::string> args_map;

  yarp::dev::IFrameGrabberControls* grabber{nullptr};
  UltraPythonCli client(grabber);

  if (!client.ParseArgs(argc, argv, args_map)) {
    return -1;
  }
  if (!client.InitYarpCommunication(args_map["--remote"])) {
    return -1;
  }

  if (args_map.find("--set") != args_map.end()) {
    std::vector<std::string> set_args = client.splitString(args_map["--set"], "=");

    // Assume control code as first string
    try {
      control_code = std::stoi(set_args[0]);
    } catch (const std::exception& e) {
      std::cout << e.what()
                << "\nControl codes can be expressed only in integer values."
                << std::endl;
      return -1;
    }

    // Assume desired value as second string
    try {
      value = std::stoi(set_args[1]);
    } catch (const std::exception& e) {
      std::cout << e.what() << "\nInvalid set value." << std::endl;
      return -1;
    }

    bool result = grabber->setFeature(control_code, value);

    if (!result) {
      std::cout << "Unable to set control " + std::to_string(control_code) +
                       "=" + std::to_string(value) +
                       ".\nCheck remote yarpdev device."
                << std::endl;
      return -1;
    }
  }

  if (args_map.find("--get") != args_map.end()) {
    try {
      control_code = std::stoi(args_map["--get"]);
    } catch (const std::exception& e) {
      std::cout << e.what()
                << "\nControl codes can be expressed only in integer values."
                << std::endl;
      return -1;
    }
    bool result = grabber->getFeature(control_code, &value);

    if (!result) {
      std::cout << "Unable to get control " + std::to_string(control_code) +
                       ".\nCheck remote yarpdev device."
                << std::endl;
      return -1;
    }
    std::cout << "Value for control code " << control_code << " is: " << value
              << std::endl;
  }

  return 0;
}
