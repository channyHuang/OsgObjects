CMAKE_MINIMUM_REQUIRED(VERSION 3.16...4.0)

project(osgImguiOpengl)

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Eigen3 REQUIRED)

FILE(GLOB FILES_C "*.cpp" ./glad/*.*)
FILE(GLOB FILES_H "*.h" "*.inl")

add_definitions(-DGLSL_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)

set(OUTER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../commonClass)

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${OUTER_DIR}
	${EIGEN3_INCLUDE_DIR}
	${GLM_INCLUDE_DIRS}
)
	
add_subdirectory(${OUTER_DIR}/imgui imgui)
add_subdirectory(${OUTER_DIR}/commonOsg commonOsg)
add_subdirectory(${OUTER_DIR}/nativefiledialog nativefiledialog)

message(${OPENSCENEGRAPH_LIBRARIES})
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
	${OPENSCENEGRAPH_LIBRARIES}
	${GLM_LIBRARIES}
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${PROJECT_NAME}
	GL
	glfw
	${OPENSCENEGRAPH_LIBRARIES}
)
else()
target_link_libraries(${PROJECT_NAME}
	opengl32
	glfw3
)
endif()