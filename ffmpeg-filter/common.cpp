#include "common.h"

cv::Mat frame2CvFrame(AVFrame* frame, const std::string& windowName) {
    int width = frame->width;
    int height = frame->height;
    cv::Mat cvFrame = cv::Mat(height, width, CV_8UC3);
    int cvLinesizes[1];
    cvLinesizes[0] = cvFrame.step1();
    SwsContext* conversion = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format, frame->width, frame->height, AVPixelFormat::AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(conversion, frame->data, frame->linesize, 0, frame->height, &cvFrame.data, cvLinesizes);
    sws_freeContext(conversion);

    cv::resize(cvFrame, cvFrame, cv::Size(300, 300));
    cv::imshow(windowName, cvFrame);
    cv::imwrite(windowName + ".jpg", cvFrame);

    return cvFrame;
}

AVFrame* cvFrame2frame(cv::Mat& cvFrame) {
    int width = cvFrame.cols;
    int height = cvFrame.rows;

    int cvLinesizes[1];
    cvLinesizes[0] = cvFrame.step1();

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cout << "alloc frame failed " << std::endl;
        return frame;
    }
    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_BGR24; /*AV_PIX_FMT_YUV420P*/;

    int ret = av_frame_get_buffer(frame, 32);
    if (ret < 0) {
        std::cout << "get buffer failed" << std::endl;
    }
    ret = av_frame_make_writable(frame);
    if (ret < 0) {
        std::cout << " writable failed " << std::endl;
    }

    //cv::cvtColor(cvFrame, cvFrame, cv::COLOR_BGR2YUV_I420);
    //int frame_size = width * height;
    //unsigned char* data = cvFrame.data;
    //memcpy(frame->data[0], data, frame_size);
    //memcpy(frame->data[1], data + frame_size, frame_size / 4);
    //memcpy(frame->data[2], data + frame_size * 5 / 4, frame_size / 4);

    SwsContext* conversion = sws_getContext(width, height, AVPixelFormat::AV_PIX_FMT_BGR24, width, height, (AVPixelFormat)frame->format, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    sws_scale(conversion, &cvFrame.data, cvLinesizes, 0, height, frame->data, frame->linesize);
    sws_freeContext(conversion);
    return frame;
}


