#
# Copyright (C) 2010 RobotCub Consortium
# Author: Alessandro Scalzo alessandro.scalzo@iit.it
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#

cmake_minimum_required(VERSION 2.6.4)
project(iCubInterfaceGui)

if(ICUB_USE_GTK2)
    add_subdirectory(iCubInterfaceGuiClient)
else(ICUB_USE_GTK2)
    message(STATUS "GTK2 not selected, skipping iCubInterfaceGuiClient")
endif(ICUB_USE_GTK2)

#add_subdirectory(iCubInterfaceGuiLib)
#add_subdirectory(iCubInterfaceGuiServer)
