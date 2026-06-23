CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

project(osgProcessModel)

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost REQUIRED)
find_package(CGAL REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(VCG_INCLUDE_DIRS "/home/channy/Documents/thirdlibs/vcglib" CACHE PATH "")

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${CGAL_INCLUDE_DIRS}
	${OpenCV_INCLUDE_DIRS}
	${VCG_INCLUDE_DIRS}

	${OUTER_DIR}
	)
	

link_directories(${PROJECT_NAME}
	${GLFW3_LIBRARY_DIR}
	${OPENSCENEGRAPH_LIBRARIES}
	${Boost_LIBRARY_DIR}
	)

add_executable(${PROJECT_NAME}
${FILES_C}
${FILES_H}
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${PROJECT_NAME}
	imgui
	commonOsg
	commonGeometry
	nativefiledialog
	Eigen3::Eigen

	GL
	glfw
	GLU
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}

	${GMP_LIBRARIES} 
)
elseif(WIN32)
target_link_libraries(${PROJECT_NAME}
	imgui
	commonOsg
	commonGeometry
	nativefiledialog
	Eigen3::Eigen

	opengl32
	glfw3
	glu32
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}
)
endif()