CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

project(osgGDAL)

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)
find_package(GDAL REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../commonClass)

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${GDAL_INCLUDE_DIRS}

	${COMMON_DIR}
)

add_subdirectory(${COMMON_DIR}/imgui imgui)
add_subdirectory(${COMMON_DIR}/commonOsg commonOsg)
add_subdirectory(${COMMON_DIR}/nativefiledialog nativefiledialog)
	
link_directories(${PROJECT_NAME}
	${GLFW3_LIBRARY_DIR}
	${OPENSCENEGRAPH_LIBRARIES}
)

add_executable(${PROJECT_NAME}
	${FILES_C}
	${FILES_H}
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${PROJECT_NAME}
	imgui
	commonOsg
	nativefiledialog
	GL
	glfw
	${OPENSCENEGRAPH_LIBRARIES}

	GDAL::GDAL
)
else()
target_link_libraries(${PROJECT_NAME}
	imgui
	commonOsg
	nativefiledialog
	plySpecial
	opengl32
	glfw3
	${OPENSCENEGRAPH_LIBRARIES}
)
endif()
