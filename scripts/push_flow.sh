#!/bin/bash
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

# rtsp-server
# wget https://github.com/aler9/rtsp-simple-server/releases/download/v0.17.7/rtsp-simple-server_v0.17.7_linux_amd64.tar.gz
# tar xvf rtsp-simple-server_v0.17.7_linux_amd64.tar.gz
# ./rtsp-simple-server
ffmpeg -re -i ./resource/KTMR2R.mp4 -r 25 -c copy -f rtsp rtsp://localhost:8554/mystream


