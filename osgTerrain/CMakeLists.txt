CMAKE_MINIMUM_REQUIRED(VERSION 4.0)

project(osgTerrain)

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}

	${CMAKE_CURRENT_SOURCE_DIR}/../../commonClass
)
	
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
	nativefiledialog
	opengl32
	glfw3
	${OPENSCENEGRAPH_LIBRARIES}
)
