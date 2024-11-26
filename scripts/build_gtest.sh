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

# get source code
git clone https://github.com/google/googletest.git ./third_party/googletest/ || { echo "git clone failed"; exit 1; }
cd third_party
mv googletest googletest-src
mkdir googletest-1.12.0
cd googletest-1.12.0
install_path=$(pwd)
cd ..

# build
cd googletest-src
git checkout release-1.12.0
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX="$install_path" .. || { echo "cmake execution failed"; exit 1; }
make -j9 || { echo "make execution failed"; exit 1; }
make install || { echo "make install failed"; exit 1; }
