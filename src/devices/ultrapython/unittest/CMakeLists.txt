# Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.
#

project(ultrapythonunittest)

set(SOURCES gtest_main.cc
            testUltraPythonGeneral.cpp
            testControlExposition.cpp
            testControlGain.cpp
            testControlBrigthness.cpp
            testControlContrast.cpp
            testUltraPytonRunning.cpp
            testDefalutOrNotUsedParam.cpp
            testCli.cpp
            ../InterfaceForCApi.h
            ../UltraPythonCameraHelper.h
            ../UltraPythonCameraHelper.cpp
            ../../../cli/UltraPythonCli.h
            ../../../cli/UltraPythonCli.cpp
            ../Statistics.cpp
            )

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE   gmock 
                                                gtest 
                                                ultrapythoncommon  
                                                ${UDev_LIBRARIES} 
                                                YARP::YARP_os
                                                YARP::YARP_init
                                                YARP::YARP_dev
                                                YARP::YARP_conf       
                        )

# install
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )