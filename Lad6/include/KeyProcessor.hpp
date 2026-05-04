#pragma once

enum class Mode {
    NORMAL = 0,
    INVERT,
    GAUSSIAN_BLUR,
    CANNY,
    SOBEL,
    BINARIZE,
    ARROWS,
    ARROWS_ROTATE,
    ZOOM_ARROWS,
    GLITCH,
    DRAW,
    ZOOM_WHEEL,
    CROSSHAIR,
    TEXT_INFO,
    IMAGE_OVERLAY,
    SLIDER_BRIGHTNESS,
    COUNT
};

class KeyProcessor {
public:
    KeyProcessor();
    void processKey(int key);
    Mode getMode() const;
    int getBrightness() const;

private:
    Mode currentMode;
    int brightness;
};
