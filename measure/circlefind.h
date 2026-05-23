#ifndef CIRCLE_FIND
#define CIRCLE_FIND


#include <opencv2/opencv.hpp>

#include "calipertool.h"




/*
*找圆算子
*需要知道圆的大致位置
*/
class CircleFind{
public:
    /*
    *@param center圆中心点
    *@param radius圆半径
    *@param num搜索点数
    */
    CircleFind(const cv::Point2d center, float radius, int num);

    //points中x、y小于0表示无效点
    struct Circle
    {
        cv::Point2d center;
        float radius;
        std::vector<cv::Point2d> points;

        void clear(){
            center=cv::Point2d(-1, -1);
            radius=0.0;
            points.clear();
        }
    };

    /*
    *影响卡尺搜索方向
    *OUTER2INNER由圆心指向圆周向量
    *OUTER2INNER由圆周指向圆心向量
    */
    enum SearchDir{OUTER2INNER, INNER2OUTER};
    
    /*
    *搜索圆，使用CaliperTool搜索点，其中从0度位置开始逆时针摆放卡尺
    *@param mat传入图像，灰度图
    *@param dir搜索方向
    *@param polar卡尺极性
    *@param edgeLength卡尺边缘过渡长度
    *@param threshold卡尺阈值
    *@param projectLen卡尺投影长度
    *@param searchLen卡尺搜索长度
    */
    Circle measure(const cv::Mat& mat, SearchDir dir, CaliperTool::Polar polar, int edgeLength, int threshold, int projectLen, int searchLen);

    //绘制测量结果
    void drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness=1);

public:
    cv::Point2d center;
    float radius;
    int num;

private:
    SearchDir _dir;
    Circle _res;
    std::vector<CaliperTool> _calipers;
};

#endif