//
// Created by 周绍龙 on 2017/11/24.
//

#ifndef ARMORDETECTION2_0_ARMORDETECTION_HPP
#define ARMORDETECTION2_0_ARMORDETECTION_HPP

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class ArmorDetection {
private:
    Mat src;
    Point2f currentCenter;  //当前检测的中心点
    Point2f lastCenter;     //上一次检测的中心点
    vector<RotatedRect> minRects;    //轮廓
    int lost;               //检测失败的计数值，

public:
    ArmorDetection();
    explicit ArmorDetection(Mat &input);
    void setInputImage(Mat input);
    void Pretreatment();
    Point2f GetArmorCenter();

private:
    void LostTarget();
    double Distance(Point2f,Point2f);
    double max(double, double);
    double min(double, double);


};


#endif //ARMORDETECTION2_0_ARMORDETECTION_HPP
