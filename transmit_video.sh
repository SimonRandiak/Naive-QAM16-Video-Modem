#!/bin/bash

ffmpeg -re -i video.mp4   -c:v libx264 -preset ultrafast -tune zerolatency   -b:v 400k -maxrate 400k -bufsize 200k -g 30   -x264-params "intra-refresh=1:slice-max-size=1200"   -an -f mpegts "udp://127.0.0.1:1234?pkt_size=1316"
