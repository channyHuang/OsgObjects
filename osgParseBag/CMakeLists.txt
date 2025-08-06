CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

set(projectName osgParseBag)
project(${projectName})

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)
find_package(OpenCV REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(OUTER_DIR ${CMAKE_SOURCE_DIR}/../commonClass)

include_directories(${projectName}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${OpenCV_INCLUDE_DIRS}

	${OUTER_DIR}
	)

add_subdirectory(${OUTER_DIR}/imgui imgui)
add_subdirectory(${OUTER_DIR}/commonOsg commonOsg)
add_subdirectory(${OUTER_DIR}/nativefiledialog nativefiledialog)

link_directories(${projectName}
	${GLFW3_LIBRARY_DIR}
	${OPENSCENEGRAPH_LIBRARIES}
	)

add_executable(${projectName}
${FILES_C}
${FILES_H}
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${projectName}
	imgui
	commonOsg
	GL
	glfw
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}
)
else()
target_link_libraries(${projectName}
	imgui
	commonOsg
	opengl32
	glfw3
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}
)
endif()