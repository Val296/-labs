#include "FrameProcessor.hpp"
#include <chrono>
#include <cmath>
#include <iostream>

FrameProcessor::FrameProcessor() {
    lastTime = std::chrono::steady_clock::now();
    // Спробуємо завантажити overlay картинку (якщо є)
    overlayImage = cv::imread("overlay.png", cv::IMREAD_UNCHANGED);
}

cv::Mat FrameProcessor::process(const cv::Mat& frame, Mode mode,
                                int brightness, MouseState& ms) {
    frameCount++;
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - lastTime).count();
    if (elapsed >= 1.0) {
        fps = frameCount / elapsed;
        frameCount = 0;
        lastTime = now;
    }

    cv::Mat result;
    switch (mode) {
        case Mode::NORMAL:            result = applyNormal(frame);        break;
        case Mode::INVERT:            result = applyInvert(frame);        break;
        case Mode::GAUSSIAN_BLUR:     result = applyGaussianBlur(frame);  break;
        case Mode::CANNY:             result = applyCanny(frame);         break;
        case Mode::SOBEL:             result = applySobel(frame);         break;
        case Mode::BINARIZE:          result = applyBinarize(frame);      break;
        case Mode::ARROWS:            result = applyArrows(frame);        break;
        case Mode::ARROWS_ROTATE:     result = applyArrowsRotate(frame.clone()); break;
        case Mode::ZOOM_ARROWS:       result = applyZoomArrows(frame.clone());   break;
        case Mode::GLITCH:            result = applyGlitch(frame.clone());       break;
        case Mode::DRAW:              result = applyDraw(frame.clone(), ms);     break;
        case Mode::ZOOM_WHEEL:        result = applyZoomWheel(frame, ms);        break;
        case Mode::CROSSHAIR:         result = applyCrosshair(frame.clone(), ms);break;
        case Mode::TEXT_INFO:         result = applyTextInfo(frame.clone());     break;
        case Mode::IMAGE_OVERLAY:     result = applyImageOverlay(frame.clone()); break;
        case Mode::SLIDER_BRIGHTNESS: result = applyBrightness(frame.clone(), brightness); break;
        default:                      result = frame.clone();             break;
    }
    return result;
}

cv::Mat FrameProcessor::applyNormal(const cv::Mat& f) { return f.clone(); }

cv::Mat FrameProcessor::applyInvert(const cv::Mat& f) {
    cv::Mat out;
    cv::bitwise_not(f, out);
    return out;
}

cv::Mat FrameProcessor::applyGaussianBlur(const cv::Mat& f) {
    cv::Mat out;
    cv::GaussianBlur(f, out, cv::Size(15, 15), 0);
    return out;
}

cv::Mat FrameProcessor::applyCanny(const cv::Mat& f) {
    cv::Mat gray, edges, out;
    cv::cvtColor(f, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 50, 150);
    cv::cvtColor(edges, out, cv::COLOR_GRAY2BGR);
    return out;
}

cv::Mat FrameProcessor::applySobel(const cv::Mat& f) {
    cv::Mat gray, gx, gy, absGx, absGy, out;
    cv::cvtColor(f, gray, cv::COLOR_BGR2GRAY);
    cv::Sobel(gray, gx, CV_16S, 1, 0);
    cv::Sobel(gray, gy, CV_16S, 0, 1);
    cv::convertScaleAbs(gx, absGx);
    cv::convertScaleAbs(gy, absGy);
    cv::addWeighted(absGx, 0.5, absGy, 0.5, 0, out);
    cv::cvtColor(out, out, cv::COLOR_GRAY2BGR);
    return out;
}

cv::Mat FrameProcessor::applyBinarize(const cv::Mat& f) {
    cv::Mat gray, bin, out;
    cv::cvtColor(f, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, bin, 127, 255, cv::THRESH_BINARY);
    cv::cvtColor(bin, out, cv::COLOR_GRAY2BGR);
    return out;
}

cv::Mat FrameProcessor::applyArrows(const cv::Mat& f) {
    cv::Mat out = f.clone();
    int cx = out.cols / 2, cy = out.rows / 2;
    // Малюємо 4 стрілки
    cv::arrowedLine(out, {cx, cy}, {cx + 60, cy},      {0,0,255},   3);
    cv::arrowedLine(out, {cx, cy}, {cx - 60, cy},      {0,255,0},   3);
    cv::arrowedLine(out, {cx, cy}, {cx, cy - 60},      {255,0,0},   3);
    cv::arrowedLine(out, {cx, cy}, {cx, cy + 60},      {255,255,0}, 3);
    return out;
}

cv::Mat FrameProcessor::applyArrowsRotate(cv::Mat f) {
    arrowAngle = (arrowAngle + 2) % 360;
    int cx = f.cols / 2, cy = f.rows / 2, r = 80;
    double rad = arrowAngle * CV_PI / 180.0;
    cv::Point tip(cx + r * cos(rad), cy + r * sin(rad));
    cv::arrowedLine(f, {cx, cy}, tip, {0, 200, 255}, 3);
    return f;
}

cv::Mat FrameProcessor::applyZoomArrows(cv::Mat f) {
    zoomArrowOffset = (zoomArrowOffset + 2) % 60;
    int cx = f.cols / 2, cy = f.rows / 2;
    int d = 40 + zoomArrowOffset;
    cv::arrowedLine(f, {cx, cy}, {cx + d, cy}, {0,0,255}, 3);
    cv::arrowedLine(f, {cx, cy}, {cx - d, cy}, {0,255,0}, 3);
    cv::arrowedLine(f, {cx, cy}, {cx, cy - d}, {255,0,0}, 3);
    cv::arrowedLine(f, {cx, cy}, {cx, cy + d}, {255,255,0}, 3);
    return f;
}

cv::Mat FrameProcessor::applyGlitch(cv::Mat f) {
    // Зсув каналів R/G/B
    cv::Mat channels[3];
    cv::split(f, channels);
    int shift = rand() % 20 - 10;
    cv::Mat M = (cv::Mat_<double>(2,3) << 1,0,shift, 0,1,0);
    cv::warpAffine(channels[2], channels[2], M, channels[2].size());
    M.at<double>(0,2) = -shift;
    cv::warpAffine(channels[0], channels[0], M, channels[0].size());
    cv::merge(channels, 3, f);
    return f;
}

cv::Mat FrameProcessor::applyDraw(cv::Mat f, MouseState& ms) {
    // Малюємо всі збережені лінії
    for (auto& ln : ms.lines)
        cv::line(f, ln.first, ln.second, {0, 255, 0}, 2);
    // Якщо ЛКМ натиснута — малюємо поточну лінію
    if (ms.leftDown && ms.drawStart.x >= 0)
        cv::line(f, ms.drawStart, ms.pos, {0, 100, 255}, 2);
    return f;
}

cv::Mat FrameProcessor::applyZoomWheel(const cv::Mat& f, MouseState& ms) {
    if (ms.zoomScale == 1.0) return f.clone();
    cv::Mat out;
    cv::resize(f, out, {}, ms.zoomScale, ms.zoomScale);
    // Центруємо
    cv::Mat canvas(f.size(), f.type(), cv::Scalar(0,0,0));
    int x = (canvas.cols - out.cols) / 2;
    int y = (canvas.rows - out.rows) / 2;
    if (ms.zoomScale > 1.0) {
        cv::Rect roi(
            (out.cols - f.cols)/2, (out.rows - f.rows)/2,
            f.cols, f.rows
        );
        roi &= cv::Rect(0, 0, out.cols, out.rows);
        out(roi).copyTo(canvas);
    } else {
        cv::Rect dst(std::max(x,0), std::max(y,0), out.cols, out.rows);
        out.copyTo(canvas(dst));
    }
    return canvas;
}

cv::Mat FrameProcessor::applyCrosshair(cv::Mat f, MouseState& ms) {
    cv::line(f, {0, ms.pos.y}, {f.cols, ms.pos.y}, {0,255,255}, 1);
    cv::line(f, {ms.pos.x, 0}, {ms.pos.x, f.rows}, {0,255,255}, 1);
    cv::circle(f, ms.pos, 10, {0,255,255}, 1);
    return f;
}

cv::Mat FrameProcessor::applyTextInfo(cv::Mat f) {
    // Середня інтенсивність
    cv::Mat gray;
    cv::cvtColor(f, gray, cv::COLOR_BGR2GRAY);
    double mean = cv::mean(gray)[0];

    // Середнє по каналах
    cv::Scalar chMean = cv::mean(f);

    std::string fpsStr  = "FPS: "    + std::to_string((int)fps);
    std::string meanStr = "Mean: "   + std::to_string((int)mean);
    std::string rgbStr  = "B:"       + std::to_string((int)chMean[0])
                        + " G:"      + std::to_string((int)chMean[1])
                        + " R:"      + std::to_string((int)chMean[2]);

    cv::putText(f, fpsStr,  {10, 30},  cv::FONT_HERSHEY_SIMPLEX, 0.8, {0,255,0}, 2);
    cv::putText(f, meanStr, {10, 60},  cv::FONT_HERSHEY_SIMPLEX, 0.8, {0,255,0}, 2);
    cv::putText(f, rgbStr,  {10, 90},  cv::FONT_HERSHEY_SIMPLEX, 0.8, {0,255,0}, 2);
    return f;
}

cv::Mat FrameProcessor::applyImageOverlay(cv::Mat f) {
    if (overlayImage.empty()) {
        cv::putText(f, "No overlay.png found", {10,50},
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, {0,0,255}, 2);
        return f;
    }
    cv::Mat overlay;
    cv::resize(overlayImage, overlay, f.size());
    if (overlay.channels() == 4) {
        // Alpha blending
        std::vector<cv::Mat> ch;
        cv::split(overlay, ch);
        cv::Mat alpha;
        ch[3].convertTo(alpha, CV_32F, 1.0/255);
        cv::Mat rgb;
        cv::merge(std::vector<cv::Mat>{ch[0],ch[1],ch[2]}, rgb);
        cv::Mat fFloat, rgbFloat, out32;
        f.convertTo(fFloat, CV_32FC3);
        rgb.convertTo(rgbFloat, CV_32FC3);
        cv::Mat alphaMat;
        cv::merge(std::vector<cv::Mat>{alpha,alpha,alpha}, alphaMat);
        out32 = rgbFloat.mul(alphaMat) + fFloat.mul(1.0 - alphaMat);
        out32.convertTo(f, CV_8UC3);
    } else {
        cv::addWeighted(f, 0.5, overlay, 0.5, 0, f);
    }
    return f;
}

cv::Mat FrameProcessor::applyBrightness(cv::Mat f, int brightness) {
    double alpha = brightness / 50.0; // 0..2
    f.convertTo(f, -1, alpha, 0);
    return f;
}
