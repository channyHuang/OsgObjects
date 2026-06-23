cmake_minimum_required(VERSION 4.0.0)

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

include_directories(${PROJECT_NAME}
    ${OUTER_DIR}

	${GLFW3_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
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

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${PROJECT_NAME}
	GL
	glfw
	GLEW

	imgui
	nativefiledialog
	${GLM_LIBRARIES}
	Eigen3::Eigen
)
else()
target_link_libraries(${PROJECT_NAME}
	opengl32
	glfw3

	imgui
	nativefiledialog
	${GLM_LIBRARIES}
	Eigen3::Eigen
)
endif()
