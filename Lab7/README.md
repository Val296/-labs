# Lab7 Computer Vision та багатопотоковість у C++

## Залежності
- OpenCV 4.x з модулем dnn
- CMake >= 3.10
- g++ з підтримкою C++17

## Запуск

```bash
chmod +x preinstall.sh
./preinstall.sh        # завантажує модель

mkdir build && cd build
cmake ..
make
./face_detection
```

## Управління
- `F` — увімкнути/вимкнути детекцію облич
- `Q` / `ESC` — вийти
