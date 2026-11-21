#ifndef CIRCLE_FIND
#define CIRCLE_FIND

#include <opencv2/opencv.hpp>
#include "ellipsefind.h"

/*
* 找圆算子
* 使用时需要知道目标的大致位置
*/
class CircleFind : public EllipseFind {
public:
    CircleFind() = default;

    /*
    * 创建近似圆搜索算子
    * @param center 圆中心点
    * @param radius 圆半径
    * @param num 搜索点数
    */
    CircleFind(const cv::Point2d center, float radius, int num);

    /*
    * 圆测量结果
    * 继承EllipseFind的Ellipse结果结构，并提供圆专用radius字段
    * points中x、y小于0表示卡尺未找到有效点
    */
    struct Circle : EllipseFind::Ellipse {};

    //设置搜索参考圆
    void setCircle(Circle c);

    /*
    * 影响卡尺搜索方向
    * OUTER2INNER，由外向内，由圆周指向圆心向量
    * INNER2OUTER，由内向外，由圆心指向圆周向量
    */
    enum SearchDir{OUTER2INNER=0, INNER2OUTER=1};

    // i=0时返回OUTER2INNER，其他情况返回INNER2OUTER
    static SearchDir searchDirInt(int i);
    
    /*
    * 搜索圆，使用CaliperTool搜索边缘点
    * 卡尺从0度位置开始，按图片坐标系顺时针摆放
    * @param mat传入图像，灰度图
    * @param dir搜索方向
    * @param polar卡尺极性
    * @param edgeLength卡尺边缘过渡长度
    * @param threshold卡尺阈值
    * @param projectLen卡尺投影长度
    * @param searchLen卡尺搜索长度
    * @param outputMode点输出模式
    */
    Circle measure(const cv::Mat& mat, int dir, int polar, int edgeLength, int threshold, int projectLen, int searchLen, int outputMode=0);

    //绘制定位圆、卡尺和测量圆
    void drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness=1);

public:
    Circle _res;

protected:
    cv::Point2d getCaliperPoint(int i) const override;
    float getCaliperAngle(int i) const override;
    EllipseFind::Ellipse fitValidPoints(
        const std::vector<cv::Point2d>& validPoints) override;
};

inline bool validCircle(CircleFind::Circle c) {
    return c.center.x >= 0 && c.center.y >= 0 &&
           (c.radius > 0 || (c.xRadius > 0 && c.yRadius > 0));
}

#endif