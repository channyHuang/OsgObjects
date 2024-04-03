CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

set(projectName osgOperateObj)
project(${projectName})

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)
find_package(CGAL REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(VCG_INCLUDE_DIR "" CACHE PATH "")

include_directories(${projectName}
	${OSG_INCLUDE_DIR}
	"./../commonClass"
	${GLFW3_INCLUDE_DIR}
	${EIGEN3_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${CGAL_INCLUDE_DIRS}
	${OpenCV_INCLUDE_DIRS}
	${VCG_INCLUDE_DIR}
	)
	
link_directories(${projectName}
	${GLFW3_LIBRARY_DIR}
	${OPENSCENEGRAPH_LIBRARIES}
	${Boost_LIBRARY_DIR}
	)

add_executable(${projectName}
${FILES_C}
${FILES_H}
)


if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${projectName}
	imgui
	commonOsg
	commonGeometry
	GL
	glfw
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}
)
elseif(WIN32)
target_link_libraries(${projectName}
	imgui
	commonOsg
	commonGeometry
	nativefiledialog
	opengl32
	glfw3
	glu32
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}
)
endif()