#pragma once
#include <opencv2/opencv.hpp>

class CameraProvider {
public:
    CameraProvider(int cameraIndex = 0);
    ~CameraProvider();
    cv::Mat getFrame();
    bool isOpened() const;

private:
    cv::VideoCapture cap;
};
