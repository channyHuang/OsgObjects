cmake_minimum_required(VERSION 3.0.0)

project(imguiShader)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(glm REQUIRED)

FILE(GLOB FILES_C "*.cpp" ./glad/*.*)
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#set(GLEW_INCLUDE_DIR "" CACHE PATH "")
add_definitions(-DGLSL_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

set(OUTER_DIR ${CMAKE_SOURCE_DIR}/../commonClass)
add_subdirectory(${OUTER_DIR}/imgui imgui)
add_subdirectory(${OUTER_DIR}/commonOsg commonOsg)
add_subdirectory(${OUTER_DIR}/nativefiledialog nativefiledialog) 

include_directories(${PROJECT_NAME}
    ${OUTER_DIR}

	${GLFW3_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	${EIGEN3_INCLUDE_DIR}
	${GLM_INCLUDE_DIRS}
	)
	
link_directories(${PROJECT_NAME}
	${GLFW3_LIBRARY_DIR}
	${GLEW_LIBRARY_DIR}
    )

add_executable(${PROJECT_NAME}
${FILES_C}
${FILES_H}
)

target_link_libraries(${PROJECT_NAME}
	imgui
	nativefiledialog
	${GLM_LIBRARIES}
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${PROJECT_NAME}
	GL
	glfw
	GLEW
	
)
else()
target_link_libraries(${PROJECT_NAME}
	opengl32
	glfw3
)
endif()