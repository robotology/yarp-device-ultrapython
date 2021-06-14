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
  int controlCode = 0;
  double value = 0.0;
  std::string port_name = "/grabber";
  std::map<std::string, std::string> argsMap;

  yarp::dev::IFrameGrabberControls* grabber{nullptr};
  UltraPythonCli client(grabber);

  if (!client.ParseArgs(argc, argv, argsMap)) {
    return -1;
  }
  if (!client.InitYarpCommunication(argsMap["--remote"])) {
    return -1;
  }

  if (argsMap.find("--set") != argsMap.end()) {
    std::vector<std::string> setArgs =
        client.splitString(argsMap["--set"], "=");

    // Assume control code as first string
    try {
      controlCode = std::stoi(setArgs[0]);
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl <<
          "Control codes can be expressed only in integer values." << std::endl;
      return -1;
    }

    // Assume desired value as second string
    try {
      value = std::stoi(setArgs[1]);
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl << "Invalid set value." << std::endl;
      return -1;
    }

    bool result = grabber->setFeature(controlCode, value);

    if (!result) {
      std::cout << "Unable to set control " + std::to_string(controlCode) +
                       "=" + std::to_string(value)
                << std::endl
                << ". Check remote yarpdev device." << std::endl;
      return -1;
    }
  }

  if (argsMap.find("--get") != argsMap.end()) {
    try {
      controlCode = std::stoi(argsMap["--get"]);
    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl
                << "Control codes can be expressed only in integer values."
                << std::endl;
      return -1;
    }
    bool result = grabber->getFeature(controlCode, &value);

    if (!result) {
      std::cout << "Unable to get control " + std::to_string(controlCode)
                << std::endl
                << ". Check remote yarpdev device." << std::endl;
      return -1;
    }
    std::cout << "Value for control code " << controlCode << " is: " << value
              << std::endl;
    return value;
  }

  return 0;
}
