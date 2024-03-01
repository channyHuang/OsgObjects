#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#include <libswscale/swscale.h>
}

struct FFMPEG_Fitler {
	AVFormatContext* fmt_ctx;
	AVCodecContext* dec_ctx;
	int video_stream_index = -1;

	AVFilterContext* buffersink_ctx;
	AVFilterContext* buffersrc_ctx;
	AVFilterGraph* filter_graph;
	const char* filter_descr = "null";
};

extern AVFrame* cvFrame2frame(cv::Mat& cvFrame);
extern cv::Mat frame2CvFrame(AVFrame* frame, const std::string& windowName);
