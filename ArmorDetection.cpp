//
// Created by 周绍龙 on 2017/11/24.
//

#include "ArmorDetection.hpp"

ArmorDetection::ArmorDetection() = default;

ArmorDetection::ArmorDetection(Mat &input) {
    this->src = input;

}

void ArmorDetection::setInputImage(Mat input) {
    this->src = input;
    currentCenter.x = 0;
    currentCenter.y = 0;
}

//图像预处理
void ArmorDetection::Pretreatment() {
    Mat input;
    vector<vector<Point>> temp;
    vector<Vec4i> hierarchy;
    cvtColor(src, input, CV_BGR2GRAY);
    threshold(input, input, 254, 255, CV_THRESH_BINARY);
    medianBlur(input, input, 5);
    findContours(input, temp, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    //筛选，去除一部分不合理的拟合矩形
    for (int i = 0; i < temp.size(); ++i) {
        RotatedRect minRect = minAreaRect(Mat(temp[i]));
        if (minRect.size.width > minRect.size.height) {
            minRect.angle += 90;
            float t = minRect.size.width;
            minRect.size.width = minRect.size.height;
            minRect.size.height = t;
        }

        if ((minRect.size.width * 10 > minRect.size.height)
            && (minRect.size.width * 1.5 < minRect.size.height)
            && (abs(minRect.angle) < 15)) {     //筛选条件还可以增加，以此来减少后面的处理的矩形数量
            minRects.push_back(minRect);
        }
    }

}


Point2f ArmorDetection::GetArmorCenter() {
    //遍历所有矩形，尝试两两组合
    RotatedRect leftRect, rightRect;
    vector<int *> reliability;
    double area[2], distance, height;

    if (minRects.size() < 2) {
        LostTarget();
        return currentCenter;
    }

    for (int i = 0; i < minRects.size(); ++i) {
        for (int j = i + 1; j < minRects.size(); ++j) {
            int level = 0;  //下面的判断在0的基础上增加可信度
            int temp[3];
            leftRect = minRects[i];
            rightRect = minRects[j];

            //判断两个矩形是否为左右灯条
            //主要根据矩形的 角度、面积、高度差、距离这几个条件来判断

            if (leftRect.angle == rightRect.angle) {
                level += 10;
            } else if (abs(leftRect.angle - rightRect.angle) < 5) {
                level += 8;
            } else if (abs(leftRect.angle - rightRect.angle) < 10) {
                level += 6;
            } else if (abs(leftRect.angle - rightRect.angle) < 20) {
                level += 4;
            } else if (abs(leftRect.angle - rightRect.angle) < 30) {
                level += 1;
            } else {
                break;
            }


            area[0] = leftRect.size.width * leftRect.size.height;
            area[1] = rightRect.size.width * rightRect.size.height;
            if (area[0] == area[1]) {
                level += 10;
            } else if (min(area[0], area[1]) * 1.5 > max(area[0], area[1])) {
                level += 8;
            } else if (min(area[0], area[1]) * 2 > max(area[0], area[1])) {
                level += 6;
            } else if (min(area[0], area[1]) * 2.5 > max(area[0], area[1])) {
                level += 4;
            } else if (min(area[0], area[1]) * 3 > max(area[0], area[1])) {
                level += 1;
            } else {
                break;
            }

            double half_height = (leftRect.size.height + rightRect.size.height) / 4;
            if (leftRect.center.y == rightRect.center.y) {
                level += 10;
            } else if (abs(leftRect.center.y - rightRect.center.y) < 0.2 * half_height) {
                level += 8;
            } else if (abs(leftRect.center.y - rightRect.center.y) < 0.4 * half_height) {
                level += 6;
            } else if (abs(leftRect.center.y - rightRect.center.y) < 0.8 * half_height) {
                level += 4;
            } else if (abs(leftRect.center.y - rightRect.center.y) < half_height) {
                level += 1;;
            } else {
                break;
            }

            distance = Distance(leftRect.center, rightRect.center);
            height = (leftRect.size.height + rightRect.size.height) / 2;
            if (distance != 0 && distance > height) {
                if (distance < 1.5 * height) {
                    level += 6;
                } else if (distance < 1.8 * height) {
                    level += 4;
                } else if (distance < 2.4 * height) {
                    level += 2;
                } else if (distance < 3 * height) {
                    level += 1;
                } else {
                    break;
                }
            }

            temp[0] = i;
            temp[1] = j;
            temp[2] = level;

            reliability.push_back(temp);

        }
    }

    if (reliability.empty()) {
        LostTarget();
        return currentCenter;
    } else {

        int maxLevel = 0, index = 0;
        for (int k = 0; k < reliability.size(); ++k) {
            if (reliability[k][2] > maxLevel) {
                maxLevel = reliability[k][2];
                index = k;
            }
        }

        currentCenter.x = (minRects[reliability[index][0]].center.x + minRects[reliability[index][1]].center.x) / 2;
        currentCenter.y = (minRects[reliability[index][0]].center.y + minRects[reliability[index][1]].center.y) / 2;

        //与上一次的结果对比
        if (lastCenter.x == 0 && lastCenter.y == 0) {
            lastCenter = currentCenter;
            lost = 0;
        } else {
            double difference = Distance(currentCenter, lastCenter);
            if (difference > 300) {
                LostTarget();
                return currentCenter;
            }
        }

        return currentCenter;
    }
}

void ArmorDetection::LostTarget() {
    lost++;
    if (lost < 3) {     //每秒30帧，3帧是0.1秒，最多允许使用0.1秒之前的结果
        currentCenter = lastCenter;
    } else {
        currentCenter = Point2f(0, 0);
        lastCenter = Point2f(0, 0);
    }
}

double ArmorDetection::Distance(Point2f a, Point2f b) {
    return sqrt((a.x - b.x) * (a.x - b.x) +
                (a.y - b.y) * (a.y - b.y));
}

double ArmorDetection::max(double first, double second) {
    return first > second ? first : second;
}

double ArmorDetection::min(double first, double second) {
    return first < second ? first : second;
}

