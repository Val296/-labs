#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

//  Shared state між main-потоком і DNN-потоком
struct SharedData {
    cv::Mat frame;              // поточний кадр
    cv::Mat result;             // кадр з намальованими рамками
    std::mutex mtx;
    std::condition_variable cv_new_frame;
    std::condition_variable cv_result_ready;
    std::atomic<bool> running{true};
    std::atomic<bool> face_mode{false};
    bool new_frame_available = false;
    bool result_available    = false;
};

//  Функція потоку: нейромережа
void dnn_worker(SharedData& data, cv::dnn::Net net) {
    while (data.running) {
        cv::Mat frame_copy;

        // Чекаємо новий кадр
        {
            std::unique_lock<std::mutex> lock(data.mtx);
            data.cv_new_frame.wait(lock, [&] {
                return data.new_frame_available || !data.running;
            });
            if (!data.running) break;
            frame_copy = data.frame.clone();
            data.new_frame_available = false;
        }

        // 1. Перетворити кадр у blob
        cv::Mat blob = cv::dnn::blobFromImage(
            frame_copy,
            1.0,
            cv::Size(300, 300),
            cv::Scalar(104.0, 177.0, 123.0)
        );

        // 2. Інференс
        net.setInput(blob);
        cv::Mat detections = net.forward();

        // detections має форму [1, 1, N, 7]
        cv::Mat det_mat(detections.size[2], detections.size[3], CV_32F,
                        detections.ptr<float>());

        // 3. Малюємо рамки
        int h = frame_copy.rows;
        int w = frame_copy.cols;

        for (int i = 0; i < det_mat.rows; i++) {
            float confidence = det_mat.at<float>(i, 2);

            if (confidence > 0.5f) {   // фільтр > 50%
                int x1 = static_cast<int>(det_mat.at<float>(i, 3) * w);
                int y1 = static_cast<int>(det_mat.at<float>(i, 4) * h);
                int x2 = static_cast<int>(det_mat.at<float>(i, 5) * w);
                int y2 = static_cast<int>(det_mat.at<float>(i, 6) * h);

                // Обмежуємо координати межами кадру
                x1 = std::max(0, x1); y1 = std::max(0, y1);
                x2 = std::min(w - 1, x2); y2 = std::min(h - 1, y2);

                // Рамка навколо обличчя
                cv::rectangle(frame_copy,
                              cv::Point(x1, y1), cv::Point(x2, y2),
                              cv::Scalar(0, 255, 0), 2);

                // Підпис з confidence
                std::string label = cv::format("Face %.0f%%", confidence * 100);
                cv::putText(frame_copy, label,
                            cv::Point(x1, y1 - 8),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5,
                            cv::Scalar(0, 255, 0), 1);
            }
        }

        // 4. Повертаємо результат
        {
            std::lock_guard<std::mutex> lock(data.mtx);
            data.result          = frame_copy;
            data.result_available = true;
        }
        data.cv_result_ready.notify_one();
    }
}

//  main
int main() {
    // Завантажуємо мережу
    cv::dnn::Net net = cv::dnn::readNetFromCaffe(
        "deploy.prototxt",
        "res10_300x300_ssd_iter_140000.caffemodel"
    );

    if (net.empty()) {
        std::cerr << "Помилка: не вдалося завантажити модель!\n";
        return -1;
    }

    // Відкриваємо камеру
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Помилка: камера недоступна!\n";
        return -1;
    }

    SharedData data;

    // Запускаємо DNN-потік
    std::thread worker(dnn_worker, std::ref(data), net);

    cv::Mat display_frame;

    std::cout << "Натисни F — увімкнути/вимкнути детекцію облич\n";
    std::cout << "Натисни Q або ESC — вийти\n";

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        // Обробка клавіш
        int key = cv::waitKey(1) & 0xFF;
        if (key == 'q' || key == 27) break;           // Q або ESC — вихід
        if (key == 'f' || key == 'F') {
            data.face_mode = !data.face_mode;
            std::cout << "Face mode: " << (data.face_mode ? "ON" : "OFF") << "\n";
        }

        if (data.face_mode) {
            // Надсилаємо кадр у DNN-потік
            {
                std::lock_guard<std::mutex> lock(data.mtx);
                data.frame             = frame.clone();
                data.new_frame_available = true;
            }
            data.cv_new_frame.notify_one();

            // Забираю результат (якщо готовий)
            {
                std::unique_lock<std::mutex> lock(data.mtx);
                if (data.cv_result_ready.wait_for(
                        lock,
                        std::chrono::milliseconds(30),
                        [&] { return data.result_available.load(); }
                    ))
                {
                    display_frame        = data.result.clone();
                    data.result_available = false;
                }
            }

            if (!display_frame.empty())
                cv::imshow("Camera", display_frame);
            else
                cv::imshow("Camera", frame);   // поки мережа думає

        } else {
            cv::imshow("Camera", frame);
        }
    }

    // Завершення
    data.running = false;
    data.cv_new_frame.notify_all();
    worker.join();

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
