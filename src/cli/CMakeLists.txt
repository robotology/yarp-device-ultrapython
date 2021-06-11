# Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

cmake_minimum_required(VERSION 3.5)

project(ultrapythoncli LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(ultrapythoncli)

target_link_libraries(ultrapythoncli
PRIVATE
  ultrapythoncommon
  YARP::YARP_os
  YARP::YARP_sig
  YARP::YARP_dev
)

target_sources(ultrapythoncli
PRIVATE
UltraPythonCli.h
UltraPythonCli.cpp
main.cpp
)

target_compile_features(ultrapythoncli PRIVATE cxx_std_17)