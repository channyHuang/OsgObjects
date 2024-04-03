CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

set(projectName imguiOsg)
project(${projectName})

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${projectName}
	${OSG_INCLUDE_DIR}
	"./../commonClass"
	${GLFW3_INCLUDE_DIR}
	)
	
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
)
elseif(WIN32)
target_link_libraries(${projectName}
	imgui
	commonOsg
	opengl32
	glfw3
	nativefiledialog
	${OPENSCENEGRAPH_LIBRARIES}
)
endif()

