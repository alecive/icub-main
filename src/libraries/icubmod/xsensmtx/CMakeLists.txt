#
# Copyright (C) 2008 RobotCub Consortium
# Author: Lorenzo Natale
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#

yarp_prepare_device(xsensmtx TYPE yarp::dev::XSensMTx INCLUDE XSensMTx.h WRAPPER inertial)

if (NOT SKIP_xsensmtx)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
                        ${YARP_INCLUDE_DIRS})
    yarp_add_plugin(xsensmtx MTComm.cpp  MTComm.h  XSensMTx.cpp  XSensMTx.h)
    target_link_libraries(xsensmtx ${YARP_LIBRARIES})
    icub_export_plugin(xsensmtx)
    yarp_install(FILES xsensmtx.ini  DESTINATION ${ICUB_PLUGIN_MANIFESTS_INSTALL_DIR})
endif (NOT SKIP_xsensmtx)
