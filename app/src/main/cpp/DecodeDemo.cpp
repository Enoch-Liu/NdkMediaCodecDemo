//
// Created by alin on 2021/12/15.
//

#include "DecodeDemo.h"
#include "android/log.h"
#include <fcntl.h>
#include <cstdio>
#include <string>
#include <errno.h>
#include <unistd.h>

using std::string;
#define LOG_TAG "DECODE_DEMO"
#define AVLOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)    // 定义LOGD类型
#define AVLOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)     // 定义LOGI类型
#define AVLOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)     // 定义LOGW类型
#define AVLOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)    // 定义LOGE类型
#define AVLOGF(...) __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)    // 定义LOGF类

DecodeDemo::~DecodeDemo() {
    Release();
}

bool DecodeDemo::Decode() {
    string filename = "/sdcard/DCIM/Camera/ssedout.mp4";
    FILE * f = fopen(filename.c_str(), "r");
    if (!f) {
        AVLOGE("Failed to open file %s, because %s", filename.c_str(), strerror(errno));
        return false;
    }
    int fd = fileno(f);
    long cur = ftell(f);
    fseek(f, 0, SEEK_END);
    long len = ftell(f);

    extractor_ = AMediaExtractor_new();
    ssize_t ret = AMediaExtractor_setDataSourceFd(extractor_, fd, cur, len);
    fclose(f);
    if (ret < 0) {
        AVLOGE("Error when set data source fd. return %d.", ret);
        AMediaExtractor_delete(extractor_);
        return false;
    }
    int track_cnt = AMediaExtractor_getTrackCount(extractor_);
    const char *info;
    for (int i = 0; i < track_cnt; i++) {
        format_ = AMediaExtractor_getTrackFormat(extractor_, i);
        AMediaFormat_getString(format_, AMEDIAFORMAT_KEY_MIME, &info);
        string str(info);
        if (str.find("video") == 0) {
            AMediaExtractor_selectTrack(extractor_, i);
            video_track_indx_ = i;
            break;
        }
        AMediaFormat_delete(format_);
        format_ = nullptr;
    }
    if (video_track_indx_ < 0) {
        AVLOGE("There is no video stream in file %s.", filename.c_str());
        AMediaExtractor_delete(extractor_);
        return false;
    }
    AMediaFormat_getString(format_, AMEDIAFORMAT_KEY_MIME, &info);
    codec_ = AMediaCodec_createDecoderByType(info);
    if (!codec_) {
        AVLOGE("Failed to create codec for %s.", info);
        return false;
    }
    AVLOGD("AMediaCodec_configure...");
    media_status_t status = AMediaCodec_configure(codec_, format_, nullptr, nullptr, 0);
    if (status) {
        AVLOGE("Error when configure mediacodec decoder, return %d", status);
        return false;
    }
    AVLOGD("AMediaCodec_start...");
    status = AMediaCodec_start(codec_);
    if (status) {
        AVLOGE("Eror when start mediacodec decoder, return %d", status);
        return false;
    }

    ret = AMediaCodec_flush(codec_);
    if (ret != AMEDIA_OK) {
        AVLOGE("Error when flush codec. return %d.", ret);
        return false;
    }
    ret = AMediaExtractor_seekTo(extractor_, 0, AMEDIAEXTRACTOR_SEEK_PREVIOUS_SYNC);
    if (ret != AMEDIA_OK) {
        AVLOGE("Error when seek to. return %d.", ret);
        return false;
    }
    sleep(2);
    bool extractor_eos = false;
    bool decoder_eos = false;
    int inputidx = 0, outputidx = 0;
    int64_t pts;
    size_t inputsize = 0;
    ssize_t samplesize = 0;
    uint8_t *input = nullptr, *output = nullptr;
    AMediaCodecBufferInfo bufferInfo;
    int tryagaincnt = 0, readtime = 0;
    while (true) {
        if (!extractor_eos) {
            inputidx = AMediaCodec_dequeueInputBuffer(codec_, 30*1000);
            if (inputidx >= 0) {
                input = AMediaCodec_getInputBuffer(codec_, inputidx, &inputsize);
                samplesize = AMediaExtractor_readSampleData(extractor_, input, inputsize);
                AVLOGD("read time = %d, samplesize = %d", readtime++, samplesize);
                if (samplesize < 0) {
                    samplesize = 0;
                    extractor_eos = true;
                    AVLOGD("Video extractor has got eof. queue EOS");
                }
                pts = AMediaExtractor_getSampleTime(extractor_);
                AMediaCodec_queueInputBuffer(codec_, inputidx, 0, samplesize, pts, extractor_eos ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
                AMediaExtractor_advance(extractor_);
            } else {
                AVLOGE("Dequeue input buffer return %d, try it again later.", inputidx);
            }
        } else {
            AVLOGE("Video extractor has got eof. do nothing.");
        }

        if (!decoder_eos) {
            outputidx = AMediaCodec_dequeueOutputBuffer(codec_, &bufferInfo, 30*1000);
            if (outputidx >= 0) {
                tryagaincnt = 0;
                AVLOGD("Output succeed.");
                if (bufferInfo.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                    decoder_eos = true;
                    AVLOGD("Decoder has got eof.");
                    return true;
                }
                output = AMediaCodec_getOutputBuffer(codec_, outputidx, nullptr);
                AMediaCodec_releaseOutputBuffer(codec_, outputidx, false);
            } else if (outputidx == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
                AVLOGD("Output buffers changed.");
            } else if (outputidx == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
                AVLOGD("Output format changed.");
                tryagaincnt = 0;
            } else if (outputidx == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                AVLOGD("Try again later. tryagaincnt = %d.", tryagaincnt);
                tryagaincnt++;
                if (tryagaincnt > 40) {
                    AVLOGE("Try again 20 times continously. consider it failed.");
                    return false;
                }
            } else {
                AVLOGD("Unexpected info code %d", outputidx);
            }
        }
    }
}

bool DecodeDemo::Release() {
    if (codec_) {
        AMediaCodec_stop(codec_);
        AMediaCodec_delete(codec_);
        codec_ = nullptr;
    }
    if (extractor_) {
        AMediaExtractor_delete(extractor_);
        extractor_ = nullptr;
    }
    if (format_) {
        AMediaFormat_delete(format_);
        format_ = nullptr;
    }
    return true;
}

bool DecodeDemo::Init() {
    AVLOGD("PPPPPPPP::Init.");
    string filename = "/sdcard/DCIM/Camera/ssedout.mp4";
    FILE * f = fopen(filename.c_str(), "r");
    if (!f) {
        AVLOGE("Failed to open file %s, because %s", filename.c_str(), strerror(errno));
        return false;
    }
    int fd = fileno(f);
    long cur = ftell(f);
    fseek(f, 0, SEEK_END);
    long len = ftell(f);

    extractor_ = AMediaExtractor_new();
    ssize_t ret = AMediaExtractor_setDataSourceFd(extractor_, fd, cur, len);
    fclose(f);
    if (ret < 0) {
        AVLOGE("Error when set data source fd. return %d.", ret);
        AMediaExtractor_delete(extractor_);
        return false;
    }
    int track_cnt = AMediaExtractor_getTrackCount(extractor_);
    const char *info;
    for (int i = 0; i < track_cnt; i++) {
        format_ = AMediaExtractor_getTrackFormat(extractor_, i);
        AMediaFormat_getString(format_, AMEDIAFORMAT_KEY_MIME, &info);
        string str(info);
        if (str.find("video") == 0) {
            AMediaExtractor_selectTrack(extractor_, i);
            video_track_indx_ = i;
            break;
        }
        AMediaFormat_delete(format_);
        format_ = nullptr;
    }
    if (video_track_indx_ < 0) {
        AVLOGE("There is no video stream in file %s.", filename.c_str());
        AMediaExtractor_delete(extractor_);
        return false;
    }
    AMediaFormat_getString(format_, AMEDIAFORMAT_KEY_MIME, &info);
    codec_ = AMediaCodec_createDecoderByType(info);
    if (!codec_) {
        AVLOGE("Failed to create codec for %s.", info);
        return false;
    }
    AVLOGD("AMediaCodec_configure...");
    media_status_t status = AMediaCodec_configure(codec_, format_, nullptr, nullptr, 0);
    if (status) {
        AVLOGE("Error when configure mediacodec decoder, return %d", status);
        return false;
    }
    AVLOGD("AMediaCodec_start...");
    status = AMediaCodec_start(codec_);
    if (status) {
        AVLOGE("Eror when start mediacodec decoder, return %d", status);
        return false;
    }
    AVLOGD("PPPPPPPP::Init finish.");
    return true;
}

bool DecodeDemo::Seek(int sec) {
    AVLOGD("PPPPPPPP::Seek.");
    if (everplay_) {
        everplay_ = false;
        int ret = AMediaCodec_flush(codec_);
        if (ret != AMEDIA_OK) {
            AVLOGE("Error when flush codec. return %d.", ret);
            return false;
        }
    }
    int ret = AMediaExtractor_seekTo(extractor_, sec * 1000 * 1000, AMEDIAEXTRACTOR_SEEK_PREVIOUS_SYNC);
    if (ret != AMEDIA_OK) {
        AVLOGE("Error when seek to. return %d.", ret);
        return false;
    }
    AVLOGD("PPPPPPPP::Seek finish.");
    return true;
}

bool DecodeDemo::Play(int sec) {
    AVLOGD("PPPPPPPP::Play.");
    everplay_ = true;
    bool extractor_eos = false;
    bool decoder_eos = false;
    int inputidx = 0, outputidx = 0;
    int64_t pts;
    size_t inputsize = 0;
    ssize_t samplesize = 0;
    uint8_t *input = nullptr, *output = nullptr;
    AMediaCodecBufferInfo bufferInfo;
    int tryagaincnt = 0, readtime = 0;
    while (true) {
        if (!extractor_eos) {
            inputidx = AMediaCodec_dequeueInputBuffer(codec_, 30*1000);
            if (inputidx >= 0) {
                input = AMediaCodec_getInputBuffer(codec_, inputidx, &inputsize);
                samplesize = AMediaExtractor_readSampleData(extractor_, input, inputsize);
                if (samplesize < 0) {
                    samplesize = 0;
                    extractor_eos = true;
                    AVLOGD("Video extractor has got eof. queue EOS");
                }
                pts = AMediaExtractor_getSampleTime(extractor_);
                AVLOGD("read time = %d, samplesize = %d, pts = %lld", readtime++, samplesize, pts);
                if (pts > sec * 1000 * 1000) {
                    AVLOGD("Has play to position.");
                    return true;
                }
                AMediaCodec_queueInputBuffer(codec_, inputidx, 0, samplesize, pts, extractor_eos ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
                AMediaExtractor_advance(extractor_);
            } else {
                AVLOGE("Dequeue input buffer return %d, try it again later.", inputidx);
            }
        } else {
            AVLOGE("Video extractor has got eof. do nothing.");
        }

        if (!decoder_eos) {
            outputidx = AMediaCodec_dequeueOutputBuffer(codec_, &bufferInfo, 30*1000);
            if (outputidx >= 0) {
                tryagaincnt = 0;
                AVLOGD("Output succeed.");
                if (bufferInfo.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
                    decoder_eos = true;
                    AVLOGD("Decoder has got eof.");
                    return true;
                }
                output = AMediaCodec_getOutputBuffer(codec_, outputidx, nullptr);
                AMediaCodec_releaseOutputBuffer(codec_, outputidx, false);
            } else if (outputidx == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
                AVLOGD("Output buffers changed.");
            } else if (outputidx == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
                AVLOGD("Output format changed.");
                tryagaincnt = 0;
            } else if (outputidx == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                AVLOGD("Try again later. tryagaincnt = %d.", tryagaincnt);
                tryagaincnt++;
                if (tryagaincnt > 40) {
                    AVLOGE("Try again 20 times continously. consider it failed.");
                    return false;
                }
            } else {
                AVLOGD("Unexpected info code %d", outputidx);
            }
        }
    }
}

bool DecodeDemo::AutoPlay() {
    return Init() && Seek() && Play() && Release();
}