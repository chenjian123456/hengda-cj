# 环境配置

## 支持设备

Atlas 500

## 支持的版本

1.3.T33.B890

版本号查询方法，在Atlas产品环境下，运行以下命令：
```bash
npu-smi info
```

## 开发环境配置
### DDK
按照《Atlas 500 软件开发指导书》中“Atlas 500 开发环境”一节配置好DDK和交叉编译环境。
> 注: 里面包含了Device的OpenCV，但不包含Host侧的OpenCV，如果需要自行交叉编译


### 解码依赖
`FFmpeg`用来对拉取`RTSP`数据流并进行解码。

- ffmpeg 4.1.4 & x264
使用我发的编译版本(我上传服务器的是这个版本)，本机存放路径推荐
```
/opt/aarch64/ffmpeg
/opt/aarch64/x264
```

需要导出 `ffmpeg` 的路径到环境变量 `FFMPEG_PATH`。修改`build.sh`中的`FFMPEG_PATH`路径，换成你自己的

```bash
export FFMPEG_PATH=/opt/aarch64/ffmpeg
```