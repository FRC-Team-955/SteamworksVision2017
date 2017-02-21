#!/bin/bash
#ffmpeg -i - -vcodec libx264 -async 1 -pix_fmt yuv420p -tune zerolatency -preset ultrafast -framerate 60 -threads 0 -f rtsp -rtsp_transport tcp rtsp://localhost:8888/live.sdp
ffmpeg -i - -vcodec libx264 -async 1 -pix_fmt yuv420p -tune zerolatency -preset ultrafast -framerate 60 -threads 0 -f http://localhost:8888/feed1.ffm

#ffmpeg -i - -vcodec libx264 -tune zerolatency -preset ultrafast -framerate 30 -threads 0 -f rtsp -rtsp_transport tcp rtsp://localhost:8888/live.sdp
#ffmpeg -i - -vcodec libx264 -pix_fmt yuv420p -tune zerolatency -preset ultrafast -framerate 30 -threads 0 -f rtsp -rtsp_transport tcp rtsp://10.20.96.1:8888/live.sdp
#ffmpeg -i /dev/video2 -vcodec libx264 -pix_fmt yuv420p -tune zerolatency -preset ultrafast -framerate 60 -f rtsp -rtsp_transport tcp rtsp://localhost:8888/live.sdp
