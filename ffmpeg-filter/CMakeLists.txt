CMAKE_MINIMUM_REQUIRED(VERSION 3.8.2)

set(projectName ffmpeg-filter)
project(${projectName})

find_package(OpenCV REQUIRED)

set(ffmpeg_DIR "" CACHE PATH "")

FILE(GLOB FILES_C "*.cpp")
FILE(GLOB FILES_H "*.h" "*.inl")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

include_directories(${projectName}
	"./../commonClass"
	${ffmpeg_DIR}/include
	${OpenCV_INCLUDE_DIRS}
	)
	
link_directories(${projectName}
	${ffmpeg_DIR}/lib
	)

add_executable(${projectName}
${FILES_C}
${FILES_H}
)

target_link_libraries(${projectName}
	avcodec
	avdevice
	avfilter
	avformat
	avutil
	postproc
	swresample
	swscale
	${OPENSCENEGRAPH_LIBRARIES}
	${OpenCV_LIBS}
)
