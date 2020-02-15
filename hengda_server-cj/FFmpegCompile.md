# FFmpage 的交叉编译
编译器确认DDK已经安装完成，且设置好`DDK_HOME`环境变量。检查命令
```
echo $DDK_HOME
```

### 交叉编译x264
```
git clone https://code.videolan.org/videolan/x264.git

cd x264

./configure --prefix=/opt/aarch64/x264 --host=aarch64-linux --cross-prefix=$DDK_HOME/toolchains/aarch64-linux-gcc6.3/bin/aarch64-linux-gnu- --enable-shared --enable-static

make -j8

sudo make install
```

### 交叉编译ffmpeg
```
./configure --prefix=/opt/aarch64/ffmpeg --target-os=linux --arch=aarch64 --enable-cross-compile --cross-prefix=$DDK_HOME/toolchains/aarch64-linux-gcc6.3/bin/aarch64-linux-gnu- --enable-shared --disable-doc --enable-libx264 --extra-cflags=-I/opt/aarch64/x264/include --extra-ldflags=-L/opt/aarch64/x264/lib --enable-gpl

make -j8

sudo make install
```

### camera
```
// 主码流
rtsp://admin:IDM8779235101@192.168.1.65:554/h265/ch1/main/av_stream

// 子码流：
rtsp://admin:IDM8779235101@192.168.1.65:554/h264/ch1/sub/av_stream
```

--------------
## 参考
- [FFmpe编译](https://bbs.huaweicloud.com/forum/thread-25834-1-1.html#/login)
