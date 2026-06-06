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
    EllipseFind(const cv::Point2d center, float semiMajor, float semiMinor, float angle, int num);

    struct Ellipse {
        cv::Point2d center = cv::Point2d(-1, -1);
        float semiMajor = 0.0f;//half of the major axis length
        float semiMinor = 0.0f;//half of the minor axis length
        float angle = 0.0f;
        std::vector<cv::Point2d> points;

        void clear() {
            center = cv::Point2d(-1, -1);
            semiMajor = 0.0f;
            semiMinor = 0.0f;
            angle = 0.0f;
            points.clear();
        }
    };

    void setEllipse(Ellipse e);

    enum SearchDir { OUTER2INNER = 0, INNER2OUTER = 1 };
    static SearchDir searchDirInt(int i);

    Ellipse measure(const cv::Mat& mat, int dir, int polar, int edgeLength, int threshold, int projectLen, int searchLen);

    void drawRes(cv::Mat& inputoutput, cv::Scalar color, int thickness = 1);

public:
    cv::Point2d center;
    float semiMajor = 0.0f;
    float semiMinor = 0.0f;
    float angle = 0.0f;
    int num = 16;

private:
    SearchDir _dir;
    Ellipse _res;
    std::vector<CaliperTool> _calipers;
};

inline bool validEllipse(EllipseFind::Ellipse e) {
    return e.center.x >= 0 && e.center.y >= 0 && e.semiMajor > 0 && e.semiMinor > 0;
}

#endif
