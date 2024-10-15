CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

project(osgReconFrame)

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(OUTER_DIR ${CMAKE_SOURCE_DIR}/../commonClass)

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${OUTER_DIR}
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

add_subdirectory(${OUTER_DIR}/imgui imgui)
add_subdirectory(${OUTER_DIR}/commonOsg commonOsg)
add_subdirectory(${OUTER_DIR}/nativefiledialog nativefiledialog)
add_subdirectory(${OUTER_DIR}/commonGeometry commonGeometry)

link_directories(${PROJECT_NAME}
	${GLFW3_LIBRARY_DIR}
	${OPENSCENEGRAPH_LIBRARIES}
	)

add_executable(${PROJECT_NAME}
${FILES_C}
${FILES_H}
)

target_link_libraries(${PROJECT_NAME}
	imgui
	commonOsg
	commonGeometry
	nativefiledialog
	${OPENSCENEGRAPH_LIBRARIES}
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME}
	GL
	glfw
	Threads::Threads
	stdc++fs
)
else()
target_link_libraries(${projectName}
	opengl32
	glfw3
)
endif()