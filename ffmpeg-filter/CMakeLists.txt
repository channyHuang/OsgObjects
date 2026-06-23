CMAKE_MINIMUM_REQUIRED(VERSION 4.0)

project(ffmpeg-filter)

find_package(OpenCV REQUIRED)

set(ffmpeg_DIR "" CACHE PATH "")

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${PROJECT_NAME}
	${ffmpeg_DIR}/include
	${OpenCV_INCLUDE_DIRS}
)
	
link_directories(${PROJECT_NAME}
	${ffmpeg_DIR}/lib
)

add_executable(${PROJECT_NAME}
${FILES_C}
${FILES_H}
)

target_link_libraries(${PROJECT_NAME}
	avcodec
	avdevice
	avfilter
	avformat
	avutil
	postproc
	swresample
	swscale
	${OpenCV_LIBS}
)
