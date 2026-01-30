CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

project(osgParseBag)

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)
find_package(OpenCV REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../commonClass)

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${OpenCV_INCLUDE_DIRS}

	${COMMON_DIR}
)

# Check C++17 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
if(COMPILER_SUPPORTS_CXX17)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  add_definitions(-DCOMPILEDWITHC17)
  message(STATUS "Using flag -std=c++17.")
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif()

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
	${OpenCV_LIBS}
)
else()
target_link_libraries(${projectName}
	imgui
	commonOsg
	nativefiledialog
	opengl32
	glfw3
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}
)
endif()