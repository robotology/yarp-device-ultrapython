# Copyright (C) 2006-2021 Istituto Italiano di Tecnologia (IIT)
# All rights reserved.
#
# This software may be modified and distributed under the terms of the
# BSD-3-Clause license. See the accompanying LICENSE file for details.

#test
include(FetchContent)
if(COMPILE_WITHUNITTEST_ULTRAPYTHON)
  FetchContent_Declare(
      googletest
      GIT_REPOSITORY "https://github.com/google/googletest"
      GIT_TAG        master
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  add_subdirectory(unittest)
  endif()

yarp_prepare_plugin(ultrapython
  CATEGORY device
  TYPE UltraPythonDriver
  INCLUDE UltraPythonDriver.h
  EXTRA_CONFIG WRAPPER=grabberDual
  DEPENDS "OpenCV_FOUND;UDev_FOUND"
  DEFAULT ON
  )

if(ENABLE_ultrapython)
  yarp_add_plugin(yarp_ultrapython)

  target_sources(yarp_ultrapython
  PRIVATE
      UltraPythonDriver.cpp
      UltraPythonDriver.h
      UltraPythonCameraLogComponent.cpp
      UltraPythonCameraLogComponent.h
      UltraPythonCameraHelper.cpp
      UltraPythonCameraHelper.h
      xilinx-v4l2-controls.h
      InterfaceForCApi.h
    )

  target_compile_features(yarp_ultrapython PRIVATE cxx_std_17)

  target_link_libraries(yarp_ultrapython
    PRIVATE
      YARP::YARP_os
      YARP::YARP_sig
      YARP::YARP_dev
      ultrapythoncommon
  )

  target_include_directories(yarp_ultrapython SYSTEM PRIVATE ${OpenCV_INCLUDE_DIRS})
  target_link_libraries(yarp_ultrapython
    PRIVATE
      ${OpenCV_LIBS}
  )

  target_include_directories(yarp_ultrapython SYSTEM PRIVATE ${UDev_INCLUDE_DIRS})
  target_link_libraries(yarp_ultrapython
    PRIVATE 
      ${UDev_LIBRARIES}
  )

  yarp_install(
    TARGETS yarp_ultrapython
    EXPORT yarp-device-ultrapython
    COMPONENT yarp-device-ultrapython
    LIBRARY DESTINATION ${YARP_DYNAMIC_PLUGINS_INSTALL_DIR}
    ARCHIVE DESTINATION ${YARP_STATIC_PLUGINS_INSTALL_DIR}
    YARP_INI DESTINATION ${YARP_PLUGIN_MANIFESTS_INSTALL_DIR}
  )

  set_property(TARGET yarp_ultrapython PROPERTY FOLDER "Plugins/Device")
endif()
