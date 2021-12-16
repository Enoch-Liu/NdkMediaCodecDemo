//
// Created by alin on 2021/12/15.
//

#ifndef HWDECODEDEMO_DECODEDEMO_H
#define HWDECODEDEMO_DECODEDEMO_H
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>


class DecodeDemo {
public:
    DecodeDemo() {}
    ~DecodeDemo();
    bool Decode();
    bool Release();
    bool Init();
    bool Seek(int sec = 0);
    bool Play(int sec = 999);
    bool AutoPlay();

private:
//    AMediaFormat *format_[2] = {nullptr, nullptr};
    AMediaFormat *format_ = nullptr;
    AMediaExtractor *extractor_ = nullptr;
    AMediaCodec *codec_ = nullptr;
    int video_track_indx_ = -1;
    bool everplay_ = false;
};


#endif //HWDECODEDEMO_DECODEDEMO_H
