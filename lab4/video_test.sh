# This file contains a set of example command lines/scripts that allow you to play a video file
# over UDP or to stream the output from a webcam over UDP.
# The first line contains the 'play' command and this should be executed on the 'host' or 'server'.
# The following two lines show example receivers. 
#   The first version is for directly using gstreamer to receive the data. This will play in full
#   screen.
#   The second version uses the egt_video application which wraps the gstreamer pipeline in a simple
#   UI. The same pipeline can be used in your EGt application once you have tested it.


###################################################################################################
## 320x240 format

# transmitter 
gst-launch-1.0 -v filesrc location=masters.avi \
! decodebin ! videoconvert ! videoscale \
! video/x-raw,format=I420,width=320,height=240,framerate=24/1 \
! videorate ! video/x-raw,framerate=20/1 \
! jpegenc quality=80 ! rtpjpegpay pt=96 \
! udpsink host="10.0.0.20" port=5000

# receiver, this will be full screen
gst-launch-1.0 -v udpsrc port=5000 caps=application/x-rtp,encoding-name=JPEG,payload=26 ! \
    rtpjpegdepay ! jpegdec ! videoscale ! videoconvert ! queue ! kmssink force-modesetting=true

# receiver, this will be via an EGT based player application
egt_video --width 320 --height 240 --pipeline "udpsrc port=5000 caps=application/x-rtp,encoding-name=JPEG,payload=26 \
    ! rtpjpegdepay ! jpegdec ! capsfilter name=vcaps caps=video/x-raw,width=320,height=240,format=I420 ! videoscale ! 
    videoconvert ! appsink name=appsink"

###########################################################################################
# webcam 320x240 format
# transmitter 
gst-launch-1.0 -v v4l2src device=/dev/video0 \
    ! videoconvert ! videoscale \
    ! video/x-raw,format=I420,width=320,height=240,framerate=20/1 \
    ! jpegenc quality=80 ! rtpjpegpay pt=96 \
    ! udpsink host="10.0.0.20" port=5000

# receiver this will be full screen
gst-launch-1.0 -v udpsrc port=5000 caps=application/x-rtp,encoding-name=JPEG,payload=26 ! \
    rtpjpegdepay ! jpegdec ! videoscale ! videoconvert ! queue ! kmssink force-modesetting=true

egt_video --width 320 --height 240 --pipeline "udpsrc port=5000 caps=application/x-rtp,encoding-name=JPEG,payload=26 \
    ! rtpjpegdepay ! jpegdec ! capsfilter name=vcaps caps=video/x-raw,width=320,height=240,format=I420 ! videoscale ! 
    videoconvert ! appsink name=appsink"