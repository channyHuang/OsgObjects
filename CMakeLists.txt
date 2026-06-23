CMAKE_MINIMUM_REQUIRED(VERSION 3.16...4.0)


# Check C++20 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX20)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
  add_definitions(-DCOMPILEDWITHC20)
  message(STATUS "Using flag -std=c++20.")
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
endif()



set(OUTER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../commonClass)
# common libraries
add_subdirectory(${OUTER_DIR}/imgui imgui)
add_subdirectory(${OUTER_DIR}/commonOsg commonOsg)
add_subdirectory(${OUTER_DIR}/nativefiledialog nativefiledialog)
add_subdirectory(${OUTER_DIR}/commonGeometry commonGeometry)
add_subdirectory(${OUTER_DIR}/xatlas xatlas)



if (CMAKE_SYSTEM_NAME MATCHES "Linux")
ADD_SUBDIRECTORY(ffmpeg-filter)

ADD_SUBDIRECTORY(imguiShader)
ADD_SUBDIRECTORY(osgClassicalShow)
ADD_SUBDIRECTORY(osgGDAL)
ADD_SUBDIRECTORY(osgImguiOpengl)
ADD_SUBDIRECTORY(osgParseBag)
ADD_SUBDIRECTORY(osgReadSpecialFile)
ADD_SUBDIRECTORY(osgReconFrame)
ADD_SUBDIRECTORY(osgShowFramePose)
ADD_SUBDIRECTORY(osgProcessModel)
ADD_SUBDIRECTORY(osgShowObjects)
#ADD_SUBDIRECTORY(osgTerrain)
ADD_SUBDIRECTORY(osgUnwrap)

else()

endif()
