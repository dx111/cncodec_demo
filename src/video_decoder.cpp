#include "video_decoder.hpp"


#define debug std::cout<<"ok......."<<__LINE__<<"\n";

Demuxer::~Demuxer() = default;


int Demuxer::open() {

    pFormatCtx = avformat_alloc_context();

    AVDictionary *options = nullptr;

    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    if (avformat_open_input(&(pFormatCtx), video_path.c_str(), 0, &options) < 0) {
        std::cout << "Could't open source file " << video_path.c_str() << "\n";
        return -1;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        std::cout << "Could not find stream information";
        return -3;
    }

    video_stream_index = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_stream_index < 0) {
        std::cout << "Could't find stream" << av_get_media_type_string(AVMEDIA_TYPE_VIDEO) << " in input file "
                  << video_path << "\n";
        return -4;
    } else {

        if (pFormatCtx->streams[video_stream_index]->codec->codec_id == AV_CODEC_ID_H264) {

//            video_type=0;
        } else if (pFormatCtx->streams[video_stream_index]->codec->codec_id == AV_CODEC_ID_H265) {
//            video_type = 0;
        } else {
            // format = -1;
        }
        /* dump input information to stderr */
//        av_dump_format(pFormatCtx, 0, video_path.c_str(), 0);
    }
    return 0;
}


video_decoder::video_decoder(
        std::string video_file_path,
        int device_id) :
        video_file_path_(std::move(video_file_path)),
        device_id_(device_id),
        demuxer(video_file_path_){
    
    init_video_create_info(createInfo);
    int ret = cnvideoDecCreate(&handle, eventCallback, &createInfo);
    if (ret != 0) {
        std::cout << __FILE__ << "," << __LINE__ << ", Call cnvideoDecCreate failed, ret \n"
                  << ret << "\n";
    } else {
        std::cout << "create decoder success. \n";
    }


}


video_decoder::~video_decoder() {

}


//bool video_decoder::cnrt_init()
//{
//    cnrtDev_t dev;
//    CNRT_CHECK(cnrtInit(0));
//    CNRT_CHECK(cnrtGetDeviceHandle(&dev, device_id_));
//    CNRT_CHECK(cnrtSetCurrentDevice(dev));
//    cnrtSetCurrentChannel((cnrtChannelType_t)(0));
//}

bool video_decoder::update_video_info(cnvideoDecSequenceInfo seq_info) {
    video_type = seq_info.codec;
    createInfo.height = seq_info.height;
    createInfo.width = seq_info.width;
    createInfo.codec = seq_info.codec;
    createInfo.inputBufNum = 5;
    // seq_info.minInputBufNum;
    createInfo.outputBufNum = 5;
    // seq_info.minOutputBufNum;

    // seq_info.minInputBufNum;
}

bool video_decoder::start_decoder() {

    int ret = cnvideoDecStart(handle, &createInfo);
    if (ret < 0) {
        std::cout << __FILE__ << "," << __LINE__ << ", Call cnvideoDecStart failed, ret " << ret << "\n";
        return false;
    }
    return true;
}

bool video_decoder::add_ref(cnvideoDecOutput *pDecOuput) {
    int ret = cnvideoDecAddReference(&handle, &pDecOuput->frame);
    if (ret < 0) {
        std::cout << __FILE__ << "," << __LINE__ << ", Call cnvideoDecAddReference failed, ret(%d)\n";
        return false;
    }
    return true;
}

bool video_decoder::test() {
    demuxer.open();

    AVPacket packet;
    (void) av_init_packet(&packet);
//    packet.data = nullptr;
//    packet.size = 0;

    int frame_id = 0;
    first_frame_ = true;

    while (demuxer.read_frame(packet) >= 0) {
        if (packet.stream_index == demuxer.get_video_index()) {
            cnvideoDecInput decinput;
            std::cout << "length: " << packet.size << "\n";
            struct timeval curTime{};
            gettimeofday(&curTime, nullptr);
            decinput.pts = curTime.tv_sec * 1000000 + curTime.tv_usec;
            decinput.flags |= CNVIDEODEC_FLAG_TIMESTAMP;
            decinput.streamBuf = packet.data;
            decinput.streamLength = (u32_t) packet.size;
            int ret_val = cnvideoDecFeedData(handle, &decinput, 10000);
        }
    }


    return false;
}

int video_decoder::init_video_create_info(cnvideoDecCreateInfo &createInfo) {
    createInfo.deviceId = device_id_;
    createInfo.instance = CNVIDEODEC_INSTANCE_AUTO;
    createInfo.codec = CNCODEC_H264;
    createInfo.pixelFmt = CNCODEC_PIX_FMT_NV21;
    // createInfo.colorSpace=CNCODEC_COLOR_SPACE_BT_709;
    createInfo.width = 0;
    createInfo.height = 0;
    // createInfo.bitDepthMinus8=0;
    createInfo.progressive = 1;
    createInfo.inputBufNum = 5;
    createInfo.outputBufNum = 5;
    createInfo.suggestedLibAllocBitStrmBufSize = 0;
    createInfo.allocType = CNCODEC_BUF_ALLOC_LIB;
    createInfo.userContext = (void *) this;
    return 0;
}

int corrupt_eventCallback(void *pData, cnvideoDecStreamCorruptInfo *info) {
    std::cout << "-------------start---------------\n";
    std::cout << " frameCount  " << info->frameCount << "\n";
    std::cout << " frameNumber  " << info->frameNumber << "\n";
    std::cout << " reserved " << info->reserved << "\n";
    std::cout << "-------------end----------------\n";
    exit(0);
}

int eventCallback(cncodecCbEventType EventType, void *pData, void *pdata1) {
    switch (EventType) {
        case CNCODEC_CB_EVENT_NEW_FRAME:
            newFrameCallback(pData, (cnvideoDecOutput *) pdata1);
            //   pContext->statistics.total_frame_count++;
            break;
        case CNCODEC_CB_EVENT_SEQUENCE:
            sequenceCallback(pData, (cnvideoDecSequenceInfo *) pdata1);
            break;
        case CNCODEC_CB_EVENT_EOS:
            eosCallback(pData, pdata1);
            break;
        case CNCODEC_CB_EVENT_SW_RESET:
        case CNCODEC_CB_EVENT_HW_RESET:
            // todo....
            //   LOG_ERROR("Decode Firmware crash Event: %d \n", EventType);
            break;
        case CNCODEC_CB_EVENT_OUT_OF_MEMORY:
            outOfMemoryCallback(pData, pdata1);
            break;
        case CNCODEC_CB_EVENT_ABORT_ERROR:
            abortErrorCallback(pData, pdata1);
            break;
        case CNCODEC_CB_EVENT_STREAM_CORRUPT:
            corrupt_eventCallback(pData, (cnvideoDecStreamCorruptInfo *) pdata1);
            break;
        default:
            //   LOG_ERROR("invalid Event: %d \n", EventType);
            break;
    }

    return 0;
}


int sequenceCallback(void *pData, cnvideoDecSequenceInfo *pFormat) {
    auto *decoder = (video_decoder *) pData;
    decoder->update_video_info(*pFormat);
    decoder->start_decoder();
    return 0;
}

int newFrameCallback(void *pData, cnvideoDecOutput *pDecOuput) {
    std::cout << "new frame \n";
    static int num = 0;
    int ret = 0;
    auto *decoder = (video_decoder *) pData;
//    ret = decoder->add_ref(pDecOuput);

    // if (pContext->chanConfig.isEnableDump)
    // {
    //     writeFrameToFile(pContext, &pDecOuput->frame);
    // }
    // exit(0);

    // exception case, when is set to 1, not all buffers are released
    // this is used to test feeddata timeout and wait eos issue.

    return 0;
}

int eosCallback(void *pData, void *pData1) {
    std::cout << " eosCallback @@@@@\n";
    auto *decoder = (video_decoder *) pData;
    // sem_post(&(pContext->eosSemaphore));

    return 0;
}

int outOfMemoryCallback(void *pData, void *pData1) {
    auto *decoder = (video_decoder *) pData;
    // sem_post(&(pContext->eosSemaphore));
    // LOG_ERROR("decoder chan[#%d] out of memory, force stop\n", pContext->chanConfig.decodeChannelId);

    return 0;
}

int abortErrorCallback(void *pData, void *pData1) {
    auto *decoder = (video_decoder *) pData;
    // sem_post(&(pContext->eosSemaphore));
    // LOG_ERROR("decoder chan[#%d] occurs abort error, force stop\n", pContext->chanConfig.decodeChannelId);

    return 0;
}

int main() {
    video_decoder a("rat.avi");
    a.test();
}