# Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

cmake_minimum_required(VERSION 3.5)

project(ultrapythonui LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(YARP REQUIRED COMPONENTS os sig dev)
find_package(Qt5 COMPONENTS Widgets REQUIRED)



if(ANDROID)
  add_library(ultrapythonui SHARED
  )
else()
  add_executable(ultrapythonui
  )
endif()

target_sources(ultrapythonui
    PRIVATE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
  )

target_link_libraries(ultrapythonui 
PRIVATE 
  Qt5::Widgets
)

target_link_libraries(ultrapythonui
PRIVATE
  YARP::YARP_os
  YARP::YARP_init
  YARP::YARP_sig
  YARP::YARP_dev
)

target_include_directories(ultrapythonui SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(ultrapythonui
PRIVATE
  ${OpenCV_LIBS}
)

target_include_directories(ultrapythonui SYSTEM PRIVATE ${UDev_INCLUDE_DIRS})
target_link_libraries(ultrapythonui
PRIVATE 
  ${UDev_LIBRARIES}
)

target_compile_features(ultrapythonui PRIVATE cxx_std_17)

install(TARGETS ultrapythonui RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
