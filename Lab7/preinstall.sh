#!/bin/bash

# Face detection model (ResNet-10 SSD)
wget -q --show-progress \
  https://raw.githubusercontent.com/opencv/opencv/master/samples/dnn/face_detector/deploy.prototxt \
  -O deploy.prototxt

wget -q --show-progress \
  https://raw.githubusercontent.com/opencv/opencv_3rdparty/dnn_samples_face_detector_20170830/res10_300x300_ssd_iter_140000.caffemodel \
  -O res10_300x300_ssd_iter_140000.caffemodel
