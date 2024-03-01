#include "common.h"

static AVFormatContext* fmt_ctx;
static AVCodecContext* dec_ctx;
int video_stream_index = -1;
AVFilterContext* buffersink_ctx;
AVFilterContext* buffersrc_ctx;
AVFilterGraph* filter_graph;
const char* filter_descr = "null";

// filters 
const char* filter_mirror = "crop=iw/2:ih:0:0,split[left][tmp];[tmp]hflip[right];[left]pad=iw*2[a];[a][right]overlay=w";
const char* filter_watermark = "movie=logo.png[wm];[in][wm]overlay=5:5[out]";
const char* filter_negate = "negate[out]";
const char* filter_edge = "edgedetect[out]";
const char* filter_split4 = "scale=iw/2:ih/2[in_tmp];[in_tmp]split=4[in_1][in_2][in_3][in_4];[in_1]pad=iw*2:ih*2[a];[a][in_2]overlay=w[b];[b][in_3]overlay=0:h[d];[d][in_4]overlay=w:h[out]";
const char* filter_vintage = "curves=vintage";
const char* filter_brightness = "eq=brightness=0.5[out] ";    //亮度。The value must be a float value in range -1.0 to 1.0. The default value is "0". 
const char* filter_contrast = "eq=contrast=1.5[out] ";        //对比度。The value must be a float value in range -2.0 to 2.0. The default value is "1". 
const char* filter_saturation = "eq=saturation=1.5[out] ";    //饱和度。The value must be a float in range 0.0 to 3.0. The default value is "1". 
const char* filter_bilateral = "bilateral=sigmaS=3:sigmaR=0.3[out]";
const char* filter_cas = "cas[out]";
const char* filter_chromanr = "chromanr[out]";
const char* filter_dctdnoiz = "dctdnoiz=4.5[out]";
const char* filter_fftdnoiz = "fftdnoiz[out]";
const char* filter_fftfilt = "fftfilt=dc_Y=0:weight_Y=\'1+squish(1-(Y+X)/100)\'[out]";
const char* filter_nlmeans = "nlmeans[out]";
const char* filter_removegrain = "removegrain[out]";
const char* filter_sobel = "sobel[out]";
const char* filter_vaguedenoiser = "vaguedenoiser[out]";

int open_input_file(const char* filename)
{
    int ret;
    const AVCodec* dec;

    if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    /* select the video stream */
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        return ret;
    }
    video_stream_index = ret;

    /* create decoding context */
    dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx)
        return AVERROR(ENOMEM);
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);

    /* init the video decoder */
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }

    return 0;
}

int init_filters(const char* filters_descr)
{
    char args[512];
    int ret = 0;
    const AVFilter* buffersrc = avfilter_get_by_name("buffer");
    const AVFilter* buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();
    AVRational time_base = fmt_ctx->streams[video_stream_index]->time_base;
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };

    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
        time_base.num, time_base.den,
        dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
        args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
        NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
        AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

     /*
      * The buffer source output must be connected to the input pad of
      * the first filter described by filters_descr; since the first
      * filter input label is not specified, it is set to "in" by
      * default.
      */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
        &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}



    int image_filter(std::string sInName = "E:/projects/instant-ngp/data/nerf/fox/images/0001.jpg") {
    //    cv::Mat cvFrame = cv::imread(sInName);
    //    AVFrame* frame = cvFrame2frame(cvFrame);

    //    AVFrame* filt_frame = av_frame_alloc();
    //    if (!frame || !filt_frame) {
    //        perror("Could not allocate frame");
    //        exit(1);
    //    }
    //    frame->pts = 0;
    //    frame->best_effort_timestamp = 0;
    //    frame->time_base.num = 1;
    //    frame->time_base.den = 30000;
    //    frame->sample_aspect_ratio.num = 1;
    //    frame->sample_aspect_ratio.den = 1;

    //    char args[512];
    //    memset(args, 0, sizeof(args));
    //    int ret;
    //    const AVFilter* buffersrc = avfilter_get_by_name("buffer");
    //    const AVFilter* buffersink = avfilter_get_by_name("buffersink");

    //    AVFilterInOut* inputs = avfilter_inout_alloc();
    //    AVFilterInOut* outputs = avfilter_inout_alloc();
    //    if (!inputs || !outputs)
    //    {
    //        printf("Cannot alloc input / output\n");
    //        return -1;
    //    }

    //    AVFilterGraph* filter_graph = avfilter_graph_alloc();
    //    if (!filter_graph)
    //    {
    //        printf("Cannot create filter graph\n");
    //        return -1;
    //    }

    //    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    //    snprintf(args, sizeof(args),
    //        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
    //        frame->width, frame->height, frame->format,
    //        frame->time_base.num, frame->time_base.den,
    //        frame->sample_aspect_ratio.num, frame->sample_aspect_ratio.den);

    //    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
    //        args, NULL, filter_graph);
    //    if (ret < 0) {
    //        printf("Cannot create buffer source\n");
    //        return ret;
    //    }

    //    /* buffer video sink: to terminate the filter chain. */
    //    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
    //        NULL, NULL, filter_graph);
    //    if (ret < 0) {
    //        printf("Cannot create buffer sink\n");
    //        return ret;
    //    }

    //    /* Endpoints for the filter graph. */
    //    outputs->name = av_strdup("in");
    //    outputs->filter_ctx = buffersrc_ctx;
    //    outputs->pad_idx = 0;
    //    outputs->next = NULL;

    //    inputs->name = av_strdup("out");
    //    inputs->filter_ctx = buffersink_ctx;
    //    inputs->pad_idx = 0;
    //    inputs->next = NULL;

    //    const char* filter_fftdnoiz = "fftdnoiz[out]";
    //    filter_descr = filter_fftdnoiz;

    //    if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_descr,
    //        &inputs, &outputs, NULL)) < 0)
    //        return ret;

    //    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
    //        return ret;

    //    avfilter_inout_free(&inputs);
    //    avfilter_inout_free(&outputs);

    //    if (av_buffersrc_add_frame(buffersrc_ctx, frame) < 0) {
    //        printf("Error while feeding the filtergraph\n");
    //        return -1;
    //    }
    //    filt_frame = av_frame_alloc();

    //    while (1) {
    //        ret = av_buffersink_get_frame_flags(buffersink_ctx, filt_frame, 0);
    //        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    //            break;
    //        if (ret < 0)
    //            return ret;

    //        if (filt_frame) {
    //            frame2CvFrame(filt_frame, "filter");
    //        }
    //        av_frame_unref(filt_frame);
    //    }
    //    av_frame_unref(frame);

        return 0;
    }

    int videoFilter(std::string sInName = "D:/dataset/lab/DJI_0959.MP4") {
        int ret;
        AVPacket packet;
        AVFrame* frame;
        AVFrame* filt_frame;

        cv::Mat cvFrame;

        frame = av_frame_alloc();
        filt_frame = av_frame_alloc();
        if (!frame || !filt_frame) {
            perror("Could not allocate frame");
            exit(1);
        }

        if ((ret = open_input_file(sInName.c_str())) < 0) {
            return 0;
        }

        while (av_read_frame(fmt_ctx, &packet) >= 0) {
            if (packet.stream_index == video_stream_index) {
                ret = avcodec_send_packet(dec_ctx, &packet);
                if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
                    break;
                }

                while (ret >= 0) {
                    ret = avcodec_receive_frame(dec_ctx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                        break;
                    }
                    else if (ret < 0) {
                        av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
                        return 0;
                    }

                    frame->pts = frame->best_effort_timestamp;
                    frame2CvFrame(frame, "origin");

                    init_filters(filter_descr);

                    /* push the decoded frame into the filtergraph */
                    if (av_buffersrc_add_frame(buffersrc_ctx, frame) < 0) {
                        printf("Error while feeding the filtergraph\n");
                        break;
                    }
                    filt_frame = av_frame_alloc();

                    while (1) {
                        ret = av_buffersink_get_frame_flags(buffersink_ctx, filt_frame, 0);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        if (ret < 0)
                            return ret;

                        if (filt_frame) {
                            frame2CvFrame(filt_frame, "filter");
                        }
                        av_frame_unref(filt_frame);
                    }
                    av_frame_unref(frame);
                }
            }
        }
        return 0;
    }

int main() {
    filter_descr = "fftfilt=dc_Y=0:weight_Y=\'1+squish(1-(Y+X)/100)\'[out]";
    videoFilter();

    return 0;
}

