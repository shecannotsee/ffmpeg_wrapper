# FFmpeg 库加载问题解决方案

在使用自定义编译的 FFmpeg 库时，可能会遇到`libswresample.so.3: cannot open shared object file`的错误。这通常是由于程序在运行时无法找到 FFmpeg 的共享库

## 问题描述
当运行 C++ 程序时，出现如下错误：
```bash
error while loading shared libraries: libswresample.so.3: cannot open shared object file: No such file or directory
```
这表明程序无法找到`libswresample.so.3`共享库，尽管路径中可能存在。

## 问题分析

从 ldd 输出中可以看到，libswresample.so.3 被标记为 not found：

```bash
libswresample.so.3 => not found
```

这表明程序无法在系统的库路径中找到`libswresample.so.3 文件`。解决此问题的一种常见方法是通过设置`LD_LIBRARY_PATH`环境变量来确保程序可以找到正确的库路径。

## 解决方法
临时设置`LD_LIBRARY_PATH`

`LD_LIBRARY_PATH`环境变量用于指定程序运行时查找动态库的路径。通过设置该环境变量，可以确保程序在运行时找到正确的 FFmpeg 库。

在终端中运行以下命令来临时设置 LD_LIBRARY_PATH：
```bash
    export LD_LIBRARY_PATH=/home/shecannotsee/Desktop/sheer_third_party/libraries/ffmpeg-4.3/lib:$LD_LIBRARY_PATH
```
