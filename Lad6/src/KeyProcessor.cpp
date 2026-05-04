#include "KeyProcessor.hpp"
#include <iostream>

KeyProcessor::KeyProcessor() : currentMode(Mode::NORMAL), brightness(50) {}

void KeyProcessor::processKey(int key) {
    switch (key) {
        case '0': currentMode = Mode::NORMAL;            break;
        case '1': currentMode = Mode::INVERT;            break;
        case '2': currentMode = Mode::GAUSSIAN_BLUR;     break;
        case '3': currentMode = Mode::CANNY;             break;
        case '4': currentMode = Mode::SOBEL;             break;
        case '5': currentMode = Mode::BINARIZE;          break;
        case '6': currentMode = Mode::ARROWS;            break;
        case '7': currentMode = Mode::ARROWS_ROTATE;     break;
        case '8': currentMode = Mode::ZOOM_ARROWS;       break;
        case '9': currentMode = Mode::GLITCH;            break;
        case 'd': currentMode = Mode::DRAW;              break;
        case 'z': currentMode = Mode::ZOOM_WHEEL;        break;
        case 'c': currentMode = Mode::CROSSHAIR;         break;
        case 't': currentMode = Mode::TEXT_INFO;         break;
        case 'i': currentMode = Mode::IMAGE_OVERLAY;     break;
        case 's': currentMode = Mode::SLIDER_BRIGHTNESS; break;
        case '+': if (brightness < 100) brightness += 5; break;
        case '-': if (brightness > 0)   brightness -= 5; break;
        default: break;
    }
}

Mode KeyProcessor::getMode() const { return currentMode; }
int KeyProcessor::getBrightness() const { return brightness; }
