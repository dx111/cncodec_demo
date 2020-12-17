#ifndef CN_VIDEO_DECODER_HPP
#define CN_VIDEO_DECODER_HPP

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include <string>
#include <sys/time.h>
#include <iostream>
#include <assert.h>
#include <cnrt.h>
#include <cn_codec_common.h>
#include <cn_video_dec.h>

#include "blockqueue.h"


class Demuxer {
public:
    Demuxer(std::string video_path):video_path(std::move(video_path)){
        av_register_all();
        avformat_network_init();
    }

    ~Demuxer();

    int open();

    void close(){
        avformat_close_input(&pFormatCtx);
    }

    int read_frame(AVPacket &packet){
        return av_read_frame(pFormatCtx, &packet);
    }

    int get_video_index() const{
        return video_stream_index;
    }

private:
    std::string video_path;

    int video_stream_index;

    AVFormatContext *pFormatCtx{};
};

class video_decoder {
public:
    video_decoder(std::string video_file_path, int device_id = 0);

    ~video_decoder();

    bool test();

    int init_video_create_info(cnvideoDecCreateInfo &createInfo);

    void update_video_create_info(cnvideoDecSequenceInfo seq_info);

    int start_decoder(){
        return cnvideoDecStart(handle, &createInfo);
    }

    int add_ref(cnvideoDecOutput *pDecOuput){
        return cnvideoDecAddReference(handle, &pDecOuput->frame);
    }

    bool del_ref(cnvideoDecOutput *pDecOuput){
        return cnvideoDecReleaseReference(handle, &pDecOuput->frame);
    }

private:
    int device_id_;
    
    std::string video_file_path_;

    Demuxer demuxer;

    cncodecType video_type;

    cnvideoDecoder handle;

    cnvideoDecCreateInfo createInfo;

    bool first_frame_ = true;

    BlockQueue<cnvideoDecOutput *> *queue;

};


int eventCallback(cncodecCbEventType EventType, void *pData, void *pdata1);

int sequenceCallback(void *pData, cnvideoDecSequenceInfo *pFormat);

int newFrameCallback(void *pData, cnvideoDecOutput *pDecOuput);

int eosCallback(void *pData, void *pData1);

int outOfMemoryCallback(void *pData, void *pData1);

int abortErrorCallback(void *pData, void *pData1);

#endif