#ifndef ELLIPSE_FIND
#define ELLIPSE_FIND

#include <opencv2/opencv.hpp>
#include "calipertool.h"

/*
* 椭圆检测算子
* 使用时需要知道目标的大致位置
*/
class EllipseFind {
public:
    EllipseFind() = default;

    /*
    * 创建近似圆搜索算子
    * @param center 圆中心点
    * @param radius 圆半径
    * @param num 搜索点数
    */
    EllipseFind(const cv::Point2d center, float radius, int num);

    /*
    * 创建近似椭圆搜索算子
    * @param center椭圆中心点
    * @param xRadius椭圆局部坐标系中x方向半径
    * @param yRadius椭圆局部坐标系中y方向半径
    * @param angle椭圆旋转角度，角度制，图片坐标系中顺时针为正
    * @param num搜索点数
    */
    EllipseFind(const cv::Point2d center, float xRadius, float yRadius, float angle, int num);

    /*
    * 圆/椭圆测量结果
    * points中x、y小于0表示卡尺未找到有效点
    */
    struct Ellipse {
        cv::Point2d center = cv::Point2d(-1, -1);
        float radius = 0.0f;
        float xRadius = 0.0f;
        float yRadius = 0.0f;
        float angle = 0.0f;
        std::vector<cv::Point2d> points;

        void clear() {
            center = cv::Point2d(-1, -1);
            radius = 0.0f;
            xRadius = 0.0f;
            yRadius = 0.0f;
            angle = 0.0f;
            points.clear();
        }
    };

    //设置搜索参考椭圆
    void setEllipse(Ellipse e);

    /*
    * 影响卡尺搜索方向
    * OUTER2INNER，由外向内，由圆周指向圆心向量
    * INNER2OUTER，由内向外，由圆心指向圆周向量
    */
    enum SearchDir { OUTER2INNER = 0, INNER2OUTER = 1 };

    // i=0时返回OUTER2INNER，其他情况返回INNER2OUTER
    static SearchDir searchDirInt(int i);

    /*
    * 搜索椭圆，使用CaliperTool搜索边缘点
    * 卡尺从0度位置开始，按图片坐标系顺时针摆放
    * @param mat传入图像，灰度图
    * @param dir搜索方向，参考SearchDir
    * @param params卡尺参数（极性、边缘长度、阈值、最大结果数、输出模式和投影长度）
    * 每个卡尺仅使用其首个输出点进行拟合
    * @param searchLen卡尺搜索长度
    */
    Ellipse measure(const cv::Mat& mat, int dir, const CaliperTool::Params& params,
                    int searchLen);

    //绘制定位椭圆、卡尺和测量结果
    virtual void drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness = 1);

public:
    cv::Point2d center;
    float xRadius = 0.0f;
    float yRadius = 0.0f;
    float angle = 0.0f;
    float radius = 0.0f;
    int num = 16;

public:
    SearchDir _dir;
    Ellipse _res;
    std::vector<CaliperTool> _calipers;

protected:
    /*
    * 派生类只需指定卡尺采样点、卡尺角度和拟合方式；
    * measure负责公共校验、卡尺创建、边缘搜索和有效点收集。
    */
    virtual cv::Point2d getCaliperPoint(int i) const;
    virtual float getCaliperAngle(int i) const;
    virtual Ellipse fitValidPoints(const std::vector<cv::Point2d>& validPoints);
};

inline bool validEllipse(EllipseFind::Ellipse e) {
    return e.center.x >= 0 && e.center.y >= 0 &&
           e.xRadius > 0 && e.yRadius > 0;
}

#endif
