#include <iostream>
#include <opencv2/opencv.hpp>
#include "ArmorDetection.hpp"

using namespace std;
using namespace cv;

int main() {
    ArmorDetection *armor = new ArmorDetection();
    VideoCapture capture("2.mov");
    VideoWriter out;
    Mat input, src;

    int frame = 0;
    namedWindow("a");

    if (!capture.isOpened()) {
        cout << "视屏打开失败" << endl;
        exit(1);
    } else {
        double rate = capture.get(CV_CAP_PROP_FPS);
        cout << "视屏的帧率为：" << rate << endl;
        while (1) {
            frame++;
            capture.read(input);
            if (input.empty()) {
                break;
            }
//            if (!out.isOpened()) {
//                out.open("zsl_1.mov", CV_FOURCC('D', 'I', 'V', 'X'), rate, cv::Size(input.cols / 2, input.rows), true);
//            }

            Point2f center;
            //只保留右侧部分图像
            src = input(Range(0, input.rows), Range(input.cols / 2, input.cols));
            armor->setInputImage(src);
            armor->Pretreatment();
            center = armor->GetArmorCenter();
            cout << center << endl;

            if (center != Point2f(0, 0)) {
                circle(src, center, 10, CV_RGB(0, 0, 0), 2);
            }
            imshow("a", src);
//            out.write(src);

            if (waitKey(30) >= 0) {
                break;
            }

        }
    }

    capture.release();
    destroyWindow("a");
    return 0;
}