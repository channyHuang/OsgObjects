CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

project(osgUnwrap)

find_package(OpenSceneGraph REQUIRED osg osgViewer osgUtil osgDB osgGA osgText)
find_package(glfw3 REQUIRED)
find_package(CGAL REQUIRED)
find_package(Assimp REQUIRED)
find_package(Boost REQUIRED COMPONENTS iostreams program_options system serialization OPTIONAL_COMPONENTS ${Boost_EXTRA_COMPONENTS})

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${PROJECT_NAME}
	${OSG_INCLUDE_DIR}
	${GLFW3_INCLUDE_DIR}
	${CGAL_INCLUDE_DIRS}
	${Boost_INCLUDE_DIR}
	${ASSIMP_INCLUDE_DIRS}
	
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
	xatlas
	nativefiledialog
	${ASSIMP_LIBRARIES}
	${Boost_LIBRARIES}
	${OPENSCENEGRAPH_LIBRARIES}

	${GMP_LIBRARIES} 
)
else()
target_link_libraries(${PROJECT_NAME}
	imgui
	commonOsg
	nativefiledialog
	opengl32
	glfw3
	xatlas
	${ASSIMP_LIBRARIES}
	${OPENSCENEGRAPH_LIBRARIES}
)
endif()