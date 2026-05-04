#pragma once
#include <opencv2/opencv.hpp>
#include "KeyProcessor.hpp"

struct MouseState {
    cv::Point pos       = {0, 0};
    bool      leftDown  = false;
    double    zoomScale = 1.0;
    cv::Point crosshair = {0, 0};
    std::vector<std::pair<cv::Point,cv::Point>> lines;
    cv::Point drawStart = {-1,-1};
};

class FrameProcessor {
public:
    FrameProcessor();
    cv::Mat process(const cv::Mat& frame, Mode mode,
                    int brightness, MouseState& ms);

private:
    cv::Mat overlayImage;
    int     arrowAngle = 0;
    int     zoomArrowOffset = 0;
    int     frameCount = 0;
    double  fps = 0;
    std::chrono::steady_clock::time_point lastTime;

    cv::Mat applyNormal        (const cv::Mat& f);
    cv::Mat applyInvert        (const cv::Mat& f);
    cv::Mat applyGaussianBlur  (const cv::Mat& f);
    cv::Mat applyCanny         (const cv::Mat& f);
    cv::Mat applySobel         (const cv::Mat& f);
    cv::Mat applyBinarize      (const cv::Mat& f);
    cv::Mat applyArrows        (const cv::Mat& f);
    cv::Mat applyArrowsRotate  (cv::Mat f);
    cv::Mat applyZoomArrows    (cv::Mat f);
    cv::Mat applyGlitch        (cv::Mat f);
    cv::Mat applyDraw          (cv::Mat f, MouseState& ms);
    cv::Mat applyZoomWheel     (const cv::Mat& f, MouseState& ms);
    cv::Mat applyCrosshair     (cv::Mat f, MouseState& ms);
    cv::Mat applyTextInfo      (cv::Mat f);
    cv::Mat applyImageOverlay  (cv::Mat f);
    cv::Mat applyBrightness    (cv::Mat f, int brightness);
};
