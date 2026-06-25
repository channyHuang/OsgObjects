CMAKE_MINIMUM_REQUIRED(VERSION 4.0)

project(osgTerrain)

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText osgShadow)
find_package(glfw3 REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenMP REQUIRED)

FILE(GLOB_RECURSE FILES_C "*.cpp")
FILE(GLOB FILES_H ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h "*.inl")
#FILE(GLOB FILES_C *.cpp)
#FILE(GLOB FILES_H *.h "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}

	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/includes/
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

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${PROJECT_NAME}
	imgui
	commonOsg
	nativefiledialog
	commonFunc
	commonGeometry
	xatlas
	Eigen3::Eigen

	OpenMP::OpenMP_CXX

	GL
	glfw
	
	${OPENSCENEGRAPH_LIBRARIES}
)
else()
target_link_libraries(${projectName}
	imgui
	commonOsg
	nativefiledialog
	commonFunc
	Eigen3::Eigen

	opengl32
	glfw3
)
endif()
