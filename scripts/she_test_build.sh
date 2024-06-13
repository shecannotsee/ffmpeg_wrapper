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
# Check dir third_party
echo "Check third_party exist?"
if [ -d "./third_party" ];
then
echo "third_party."
else
echo "Create third_party."
mkdir third_party
fi

git clone https://github.com/shecannotsee/she_test.git ./third_party/she_test/

cd third_party/she_test
# build
mkdir include
mv ./src/* ./include
find . -mindepth 1 -maxdepth 1 ! -name include -exec rm -rf {} +
