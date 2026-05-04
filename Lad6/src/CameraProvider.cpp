#include "CameraProvider.hpp"
#include <stdexcept>

CameraProvider::CameraProvider(int cameraIndex) {
    cap.open(cameraIndex);
    if (!cap.isOpened())
        throw std::runtime_error("Cannot open camera");
}

CameraProvider::~CameraProvider() {
    cap.release();
}

cv::Mat CameraProvider::getFrame() {
    cv::Mat frame;
    cap >> frame;
    return frame;
}

bool CameraProvider::isOpened() const {
    return cap.isOpened();
}
