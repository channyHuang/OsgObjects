cmake_minimum_required(VERSION 3.0.0)

set(projectName imguiShader)
project(${projectName})

find_package(GLFW3 REQUIRED)
#find_package(GLEW REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${projectName}
    "./../commonClass"
	${GLFW3_INCLUDE_DIR}
	)
	
link_directories(${projectName}
	${GLFW3_LIBRARY_DIR}
    )

add_executable(${projectName}
${FILES_C}
${FILES_H}
)

target_link_libraries(${projectName}
	imgui
	opengl32
	glfw3
	nativefiledialog
)