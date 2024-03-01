CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

set(projectName osgUnwrap)
project(${projectName})

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(GLFW3 REQUIRED)
find_package(CGAL REQUIRED)
find_package(Assimp REQUIRED)
find_package(Boost REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${projectName}
	${OSG_INCLUDE_DIR}
	"./../commonClass"
	${GLFW3_INCLUDE_DIR}
	${CGAL_INCLUDE_DIRS}
	${Boost_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIRS}
	)
	
link_directories(${projectName}
	${GLFW3_LIBRARY_DIR}
	${OPENSCENEGRAPH_LIBRARIES}
	)

add_executable(${projectName}
${FILES_C}
${FILES_H}
)

target_link_libraries(${projectName}
	imgui
	commonOsg
	opengl32
	glfw3
	xatlas
	${ASSIMP_LIBRARIES}
	${OPENSCENEGRAPH_LIBRARIES}
)
