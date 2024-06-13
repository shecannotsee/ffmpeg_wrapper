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

git clone https://github.com/shecannotsee/GSL-4.0.0.git ./third_party/GSL-4.0.0/
mkdir ./third_party/gsl_4_0_0

cd ./third_party/
mv ./GSL-4.0.0/include/gsl ./gsl_4_0_0
rm -rf ./GSL-4.0.0/
