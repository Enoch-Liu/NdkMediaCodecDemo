# NdkMediaCodecDemo
Android native mediacodec decode/encode demo

This is a very light demo. So it doesn't have a presentation view. You can get play result by the text shown or by the log printed.

Video input is defined at line 175 of file app/main/cpp/DecodeDemo.cpp, you can change it to your own video file path name.

Press INIT button before press PLMD/SKBG/PLED/SKMD/SEEK/PLAY buttons.

PL is short for play, or play to a particular time stamp.

MD is short for middle.

ED is short for end.

BG is short for begin.

SK is short for seek.

SEEK button means seek to begin.

PLAY button means play to eof of the file.

PLED button means play to the 90% timeline position of the video.

FLAG button means to print a log with particular characters.

AUTO button means to press INIT/SEEK/PLAY automatically.

Current State text record the state after last pressing.

Statistics State has two value, Succeed and Failed, both are the number of times of play state.
