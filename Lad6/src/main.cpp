#include "CameraProvider.hpp"
#include "KeyProcessor.hpp"
#include "FrameProcessor.hpp"
#include "Display.hpp"
#include <iostream>

MouseState mouseState;

void onMouse(int event, int x, int y, int flags, void* userdata) {
    MouseState& ms = *reinterpret_cast<MouseState*>(userdata);
    ms.pos = {x, y};

    if (event == cv::EVENT_LBUTTONDOWN) {
        ms.leftDown = true;
        ms.drawStart = {x, y};
    }
    if (event == cv::EVENT_LBUTTONUP) {
        ms.leftDown = false;
        if (ms.drawStart.x >= 0)
            ms.lines.push_back({ms.drawStart, {x, y}});
        ms.drawStart = {-1, -1};
    }
    if (event == cv::EVENT_MOUSEWHEEL) {
        if (flags > 0) ms.zoomScale = std::min(ms.zoomScale + 0.1, 3.0);
        else           ms.zoomScale = std::max(ms.zoomScale - 0.1, 0.3);
    }
}

int main() {
    try {
        CameraProvider camera(0);
        KeyProcessor   keyProc;
        FrameProcessor frameProc;
        Display        display("Lab3 - Camera");

        // Trackbar для яскравості (слайдер)
        int brightness = 50;
        cv::createTrackbar("Brightness", display.getWindowName(),
                           &brightness, 100);

        cv::setMouseCallback(display.getWindowName(), onMouse, &mouseState);

        std::cout << "Keys: 0-9, d, z, c, t, i, s | ESC = exit\n";

        while (true) {
            cv::Mat frame = camera.getFrame();
            if (frame.empty()) break;

            int key = cv::waitKey(30) & 0xFF;
            if (key == 27) break; // ESC
            keyProc.processKey(key);

            cv::Mat result = frameProc.process(
                frame, keyProc.getMode(), brightness, mouseState
            );

            display.show(result);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
