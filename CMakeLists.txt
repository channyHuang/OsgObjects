CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

set(projectName AllOsgProj)
project(${projectName})

ADD_SUBDIRECTORY(commonClass)
ADD_SUBDIRECTORY(imguiOsg)
ADD_SUBDIRECTORY(imguiShader)
ADD_SUBDIRECTORY(osgForTempTest)

ADD_SUBDIRECTORY(osgParseBag)
ADD_SUBDIRECTORY(osgReconFrame)
#ADD_SUBDIRECTORY(osgReconRealtime)
ADD_SUBDIRECTORY(osgReconShowResult)
ADD_SUBDIRECTORY(osgShowFramePose)
ADD_SUBDIRECTORY(osgShowObjects)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
else()
ADD_SUBDIRECTORY(ffmpeg-filter)
ADD_SUBDIRECTORY(osgGDAL)
ADD_SUBDIRECTORY(osgOperateObj)
ADD_SUBDIRECTORY(osgTerrain)
ADD_SUBDIRECTORY(osgUnwrap)
ADD_SUBDIRECTORY(postRawVolume)
endif()