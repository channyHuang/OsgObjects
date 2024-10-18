cmake_minimum_required(VERSION 3.0.0)

project(imguiShader)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
#set(GLEW_INCLUDE_DIR "" CACHE PATH "")

set(OUTER_DIR ${CMAKE_SOURCE_DIR}/../commonClass)
add_subdirectory(${OUTER_DIR}/imgui imgui)
add_subdirectory(${OUTER_DIR}/commonOsg commonOsg)
add_subdirectory(${OUTER_DIR}/nativefiledialog nativefiledialog) 

include_directories(${PROJECT_NAME}
    ${OUTER_DIR}

	${GLFW3_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
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
	imgui
	GL
	glfw
	GLEW
)
else()
target_link_libraries(${PROJECT_NAME}
	imgui
	opengl32
	glfw3
	nativefiledialog
)
endif()