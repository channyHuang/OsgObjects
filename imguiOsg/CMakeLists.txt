CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

project(${imguiOsg})

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(OUTER_DIR ${CMAKE_SOURCE_DIR}/../commonClass)
add_subdirectory(${OUTER_DIR}/imgui imgui)
add_subdirectory(${OUTER_DIR}/commonOsg commonOsg)

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${OUTER_DIR}
	)
	
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
	GL
	glfw
	${OPENSCENEGRAPH_LIBRARIES}
)
elseif(WIN32)
target_link_libraries(${PROJECT_NAME}
	imgui
	commonOsg
	opengl32
	glfw3
	nativefiledialog
	${OPENSCENEGRAPH_LIBRARIES}
)
endif()

