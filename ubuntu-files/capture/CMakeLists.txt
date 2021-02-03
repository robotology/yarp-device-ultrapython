################################################################################
#                                                                              #
# Copyright (C) 2019 Fondazione Istitito Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

# @author Luca Tricerri <luca.tricerri@iit.it>

cmake_minimum_required(VERSION 3.5)

project(v4l2_capture
        LANGUAGES C CXX
        VERSION 2.3.0)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ")

set(v4l2_captureSrc  v4l2_capture.cpp
                        PythonCameraHelper.cpp
                        )
set(v4l2_captureHdr  PythonCameraHelper.h

                        )

add_executable(${PROJECT_NAME} ${v4l2_captureSrc} ${v4l2_captureHdr} )
target_include_directories(${PROJECT_NAME} PRIVATE /usr/include)
target_link_libraries(${PROJECT_NAME} PUBLIC udev)

install (TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
