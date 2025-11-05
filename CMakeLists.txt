CMAKE_MINIMUM_REQUIRED(VERSION 3.16...4.0)

project(AllOsgProj)

# ADD_SUBDIRECTORY(imguiShader)

# ADD_SUBDIRECTORY(osgParseBag)
# ADD_SUBDIRECTORY(osgReconFrame)
# ADD_SUBDIRECTORY(osgReconShowResult)
# ADD_SUBDIRECTORY(osgShowFramePose)
# ADD_SUBDIRECTORY(osgShowObjects)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
# ADD_SUBDIRECTORY(osgShowObjects)
ADD_SUBDIRECTORY(osgClassicalShow)
else()
ADD_SUBDIRECTORY(ffmpeg-filter)
ADD_SUBDIRECTORY(osgGDAL)
ADD_SUBDIRECTORY(osgImguiOpengl)
ADD_SUBDIRECTORY(osgOperateObj)
ADD_SUBDIRECTORY(osgTerrain)
ADD_SUBDIRECTORY(osgUnwrap)
ADD_SUBDIRECTORY(postRawVolume)
endif()