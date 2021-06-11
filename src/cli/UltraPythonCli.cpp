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

UltraPythonCli::UltraPythonCli(yarp::dev::IFrameGrabberControls* grabber){};

UltraPythonCli::~UltraPythonCli() {}

bool UltraPythonCli::ParseArgs(int argc, char* argv[],
                               std::map<std::string, std::string>& args_map) {
  if (argc > 5 || argc < 2) {
    std::cout << "Use 'ultrapythoncli --help'" << std::endl;
    return false;
  }

  if (std::string(argv[1]) == "--help") {
    std::cout << "Usage 'ultrapythoncli [--help] [--remote portname] "
                 "[--get controlcode] [--set "
                 "controlcode value]'\n"
                 "NOTE: the name is without rpc and "
                 "port name usually is /grabber"
              << std::endl;
    return false;
  }

  if (argc >= 3 && std::string(argv[1]) == "--remote") {
    args_map.insert({argv[1], argv[2]});
  } else {
    std::cout << "Remote port not found. Please select the appropriate remote "
                 "port or set it as first argument."
              << std::endl;
    return false;
  }

  if (std::string(argv[3]) == "--set" || std::string(argv[3]) == "--get") {
    args_map.insert({argv[3], argv[4]});
  }

  return true;
}

bool UltraPythonCli::InitYarpCommunication(const std::string& remotePort) {
  if (!yarp::os::NetworkBase::checkNetwork(2)) {
    std::cout
        << "Yarp yarpserver not found.\nPlease activate yarpserver and retry."
        << std::endl;
    return false;
  }

  property_.put("device", "remote_grabber");
  property_.put("local", "/xxx");
  property_.put("remote", remotePort + "/rpc");

  if (!device_.open(property_)) {
    return false;
  }
  device_.view(grabber_);

  if (!grabber_) {
    std::cout << "Unable to view device." << std::endl;
    return false;
  }

  return true;
}

std::vector<std::string> UltraPythonCli::splitString(const std::string& s,
                                                     const char* separator) {
  std::vector<std::string> output;
  std::string::size_type previous_position = 0;
  std::string::size_type position = 0;

  while ((position = s.find(separator, position)) != std::string::npos) {
    std::string substring(
        s.substr(previous_position, position - previous_position));
    output.push_back(substring);
    previous_position = ++position;
  }
  // Parse last word
  output.push_back(s.substr(previous_position, position - previous_position));
  return output;
}
