# Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

cmake_minimum_required(VERSION 3.5)

project(ultrapythoncommon LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


add_library(ultrapythoncommon STATIC
  )

target_sources(ultrapythoncommon
    PRIVATE
    common.h
    common.cpp
  )

target_include_directories(ultrapythoncommon PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
                                                  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>")

target_compile_features(ultrapythoncommon PRIVATE cxx_std_17)

install(TARGETS ultrapythoncommon RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
