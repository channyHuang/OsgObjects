CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

set(projectName osgReconShowResult)
project(${projectName})

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(GLFW3 REQUIRED)
#find_package(GLEW REQUIRED)
find_package(CGAL REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenCV REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")
FILE(GLOB FILES_SIFTGPU "E:/thirdLibs/colmap-dev/lib/SiftGPU/*.h" "E:/thirdLibs/colmap-dev/lib/SiftGPU/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)

include_directories(${projectName}
	${OSG_INCLUDE_DIR}
	"./../commonClass"
	${GLFW3_INCLUDE_DIR}
	${CGAL_INCLUDE_DIRS}
	${Boost_INCLUDE_DIR}
	${OpenCV_INCLUDE_DIRS}
	)
	
link_directories(${projectName}
	${GLFW3_LIBRARY_DIR}
	${OPENSCENEGRAPH_LIBRARIES}
	)

add_executable(${projectName}
${FILES_C}
${FILES_H}
#${FILES_SIFTGPU}
)

target_link_libraries(${projectName}
	imgui
	commonOsg
	opengl32
	glfw3
	nativefiledialog
	commonGeometry
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}
)
