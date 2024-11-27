#!/bin/bash
project="ffmpeg_wrapper"
project_length=${#project}
# Get pwd string
path=$(pwd)
# Get last project_length chars
suffix="${path: -$project_length}"
if [ "$suffix" = "$project" ]; then
    echo "ok"
else
    echo "Path error.Please ensure that the last few strings after using pwd are "$project
    exit 1  # stop
fi

export HTTP_PROXY="http://127.0.0.1:7890"
export HTTPS_PROXY="https://127.0.0.1:7890"
# https://gist.github.com/jsturgis/3b19447b304616f18657?permalink_comment_id=4920424#gistcomment-4920424
# get source code
wget -P ./resource http://samples.mplayerhq.hu/V-codecs/h264/NeroAVC.mp4 || \
  { echo "wget failed"; exit 1; }
ffmpeg -i ./resource/NeroAVC.mp4 -codec copy -bsf: h264_mp4toannexb -f h264 ./resource/test.h264
