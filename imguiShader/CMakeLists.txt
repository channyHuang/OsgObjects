cmake_minimum_required(VERSION 3.0.0)

set(projectName imguiShader)
project(${projectName})

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#set(GLEW_INCLUDE_DIR "" CACHE PATH "")

include_directories(${projectName}
    "./../commonClass"
	${GLFW3_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
	)
	
link_directories(${projectName}
	${GLFW3_LIBRARY_DIR}
	${GLEW_LIBRARY_DIR}
    )

add_executable(${projectName}
${FILES_C}
${FILES_H}
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
target_link_libraries(${projectName}
	imgui
	GL
	glfw
	GLEW
)
else()
target_link_libraries(${projectName}
	imgui
	opengl32
	glfw3
	nativefiledialog
)
endif()