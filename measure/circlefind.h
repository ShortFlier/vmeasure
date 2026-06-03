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

    CircleFind() = default;
    /*
    *@param center圆中心点
    *@param radius圆半径
    *@param num搜索点数
    */
    CircleFind(const cv::Point2d center, float radius, int num);

    //points中x、y小于0表示无效点
    struct Circle
    {
        cv::Point2d center= cv::Point2d(-1, -1);
        float radius=0.0;
        std::vector<cv::Point2d> points;

        void clear(){
            center=cv::Point2d(-1, -1);
            radius=0.0;
            points.clear();
        }
    };

    void setCircle(Circle c);

    /*
    *影响卡尺搜索方向
    *OUTER2INNER，由外向内，由圆周指向圆心向量
    *INNER2OUTER，由内向外，由圆心指向圆周向量
    */
    enum SearchDir{OUTER2INNER=0, INNER2OUTER=1};

    //i=0时返回OUTER2INNER，其他情况返回INNER2OUTER
    static SearchDir searchDirInt(int i);
    
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
    Circle measure(const cv::Mat& mat, int dir, int polar, int edgeLength, int threshold, int projectLen, int searchLen);

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


inline bool validCircle(CircleFind::Circle c) {
    cv::Point2d& p=c.center;
    return p.x >= 0 && p.y >= 0;
}


#endif