#ifndef ELLIPSE_FIND
#define ELLIPSE_FIND

#include <opencv2/opencv.hpp>
#include "calipertool.h"

/*
 * Ellipse finder
 * Can search using an approximate circle or ellipse initial guess
 */
class EllipseFind {
public:
    EllipseFind() = default;

    // Create with an approximate circle guess
    EllipseFind(const cv::Point2d center, float radius, int num);

    // Create with an approximate ellipse guess
    EllipseFind(const cv::Point2d center, float xRadius, float yRadius, float angle, int num);

    struct Ellipse {
        cv::Point2d center = cv::Point2d(-1, -1);
        float xRadius = 0.0f;//局部坐标x方向半径
        float yRadius = 0.0f;//局部坐标y方向半径
        float angle = 0.0f;
        std::vector<cv::Point2d> points;

        void clear() {
            center = cv::Point2d(-1, -1);
            xRadius = 0.0f;
            yRadius = 0.0f;
            angle = 0.0f;
            points.clear();
        }
    };

    //设置搜索参考椭圆
    void setEllipse(Ellipse e);

    enum SearchDir { OUTER2INNER = 0, INNER2OUTER = 1 };
    static SearchDir searchDirInt(int i);

    Ellipse measure(const cv::Mat& mat, int dir, int polar, int edgeLength, int threshold, int projectLen, int searchLen, int outputMode=0);

    void drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness = 1);

public:
    cv::Point2d center;
    float xRadius = 0.0f;
    float yRadius = 0.0f;
    float angle = 0.0f;
    int num = 16;

public:
    SearchDir _dir;
    Ellipse _res;
    std::vector<CaliperTool> _calipers;
};

inline bool validEllipse(EllipseFind::Ellipse e) {
    return e.center.x >= 0 && e.center.y >= 0 && e.xRadius > 0 && e.yRadius > 0;
}

#endif
