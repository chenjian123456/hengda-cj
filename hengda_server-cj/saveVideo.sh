#!/bin/sh
cd /root/lib/ffmpeg/bin
./ffmpeg -y -r 2 -i /root/out/result_videos_yuejie/%3d.jpg /root/out/result_videos/${1}.mp4
